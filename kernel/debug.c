#include <asm-generic/mmu.h>
#include <x86.h>
#include <stdio.h>
#include <rb_tree.h>
#include <kernel.h>
#include <assert.h>

#define N_GSYM      0x20    // global symbol
#define N_FNAME     0x22    // F77 function name
#define N_FUN       0x24    // procedure name
#define N_STSYM     0x26    // data segment variable
#define N_LCSYM     0x28    // bss segment variable
#define N_MAIN      0x2a    // main function name
#define N_PC        0x30    // global Pascal symbol
#define N_RSYM      0x40    // register variable
#define N_SLINE     0x44    // text segment line number
#define N_DSLINE    0x46    // data segment line number
#define N_BSLINE    0x48    // bss segment line number
#define N_SSYM      0x60    // structure/union element
#define N_SO        0x64    // main source file name
#define N_LSYM      0x80    // stack variable
#define N_BINCL     0x82    // include file beginning
#define N_SOL       0x84    // included source file name
#define N_PSYM      0xa0    // parameter variable
#define N_EINCL     0xa2    // include file end
#define N_ENTRY     0xa4    // alternate entry point
#define N_LBRAC     0xc0    // left bracket
#define N_EXCL      0xc2    // deleted include file
#define N_RBRAC     0xe0    // right bracket
#define N_BCOMM     0xe2    // begin common
#define N_ECOMM     0xe4    // end common
#define N_ECOML     0xe8    // end common (local name)
#define N_LENG      0xfe    // length of preceding entry

/* Entries in the STABS table are formatted as follows. */
struct stab {
    uint32_t n_strx;        // index into string table of name
    uint8_t n_type;         // type of symbol
    uint8_t n_other;        // misc info (usually empty)
    uint16_t n_desc;        // description field
    uintptr_t n_value;      // value of symbol
};

extern const struct stab __STAB_BEGIN__[];  // beginning of stabs table
extern const struct stab __STAB_END__[];    // end of stabs table
extern const char __STABSTR_BEGIN__[];      // beginning of string table
extern const char __STABSTR_END__[];        // end of string table

static struct rb_tree *g_stab_so_tree;
static struct rb_tree *g_stab_fun_tree;

void backtrace(void)
{
	unsigned long *ebp = (unsigned long *)read_ebp();
	unsigned long eip = read_eip();
	const char *str = __STABSTR_BEGIN__;
	const struct stab *stab;
	struct rb_node *node;
	const char *file = NULL, *func = NULL;
	string *s = string_create();
	char *split;
	int i;

	pr_info("Call Trace:");
	for (i = 0; ebp && i < 20; i ++) {
		node = rb_tree_search(g_stab_so_tree, eip);
		if (node) {
			stab = rb_node_value(node);
			assert_notrace(stab);
			file = str + stab->n_strx;
		} else {
			file = "<unknown>";
		}

		node = rb_tree_search(g_stab_fun_tree, eip);
		if (node) {
			stab = rb_node_value(node);
			assert_notrace(stab);

			func = str + stab->n_strx;

			s->length = 0;
			split = strfind(func, ':');
			if (split)
				string_append_strn(s, func, split - func);
			else
				string_append_str(s, func);

			pr_info("\t", s->str,
				"+", hex(eip - stab->n_value), "/",
				hex(rb_node_key_end(node) - rb_node_key_start(node)),
				"\t[", file, "]");
		} else {
			pr_info("\t", "unknown", "\t[", file, "]");
		}

		eip = ebp[1];
		ebp = (unsigned long *)ebp[0];
	}

	pr_info("---[ end trace ]---");
	string_destroy(s);
}

void debug_init(void)
{
	const struct stab *stab = __STAB_BEGIN__;
	const struct stab *prev_so_stab = NULL, *prev_fun_stab = NULL;

	g_stab_so_tree = rb_tree_create();
	g_stab_fun_tree = rb_tree_create();

	while (stab != __STAB_END__) {
		if (stab->n_type == N_SO && stab->n_strx != 0) {
			if (prev_so_stab)
				rb_tree_insert(g_stab_so_tree, prev_so_stab->n_value,
						stab->n_value - 1, (void *)prev_so_stab);
			prev_so_stab = stab;
		}

		if (stab->n_type == N_FUN) {
			if (prev_fun_stab)
				rb_tree_insert(g_stab_fun_tree, prev_fun_stab->n_value,
						stab->n_value - 1, (void *)prev_fun_stab);

			prev_fun_stab = stab;
		}

		stab++;
	}

	if (prev_so_stab)
		rb_tree_insert(g_stab_so_tree, prev_so_stab->n_value,
				0xffffffff, (void *)prev_so_stab);

	if (prev_fun_stab)
		rb_tree_insert(g_stab_fun_tree, prev_fun_stab->n_value,
				0xffffffff, (void *)prev_fun_stab);
}
