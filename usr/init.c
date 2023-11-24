#include <usr.h>

int usr_init(void)
{
	int ret;

	ret = usr_fs_init();
	if (ret)
		return ret;

	ret = usr_debug_init();
	if (ret)
		return ret;

	return 0;
}
