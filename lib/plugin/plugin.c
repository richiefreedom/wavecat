#include <dlfcn.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#define FILE_NAME_LEN 256

typedef int (*plugin_init_t)(void);

int plugin_load(const char *dir_name, const char *plugin_name)
{
	char buf[FILE_NAME_LEN];
	void *plib, *psym;
	plugin_init_t init;
	int ret;
	char *sub;

	ret = snprintf(buf, FILE_NAME_LEN, "./%s/%s",
			dir_name, plugin_name);
	if (ret >= FILE_NAME_LEN) {
		buf[FILE_NAME_LEN - 1] = '\0';
	}

	plib = dlopen(buf, RTLD_LAZY);
	if (!plib) {
		fprintf(stderr, "Unable to load the plugin '%s'\n", buf);
		return -1;
	}

	sub = strstr(plugin_name, ".so");
	if (sub)
		*sub = '\0';

	ret = snprintf(buf, FILE_NAME_LEN, "%s_init", plugin_name);
	if (ret >= FILE_NAME_LEN) {
		buf[FILE_NAME_LEN - 1] = '\0';
	}

	psym = dlsym(plib, buf);
	if (!psym) {
		fprintf(stderr, "Unable to find the plugin constructor\n");
		dlclose(plib);
		return -1;
	}

	*(void **) (&init) = psym;
	init();

	return 0;
}

int plugin_loaddir(const char *dir_name)
{
	DIR *dp;
	struct dirent *ep;
	int ret;

	dp = opendir(dir_name);
	if (!dp) {
		fprintf(stderr, "Unable to open the directory '%s'\n",
				dir_name);
		return -1;
	}

	while (ep = readdir(dp)) {
		if (!strstr(ep->d_name, ".so"))
			continue;
		ret = plugin_load(dir_name, ep->d_name);
		if (!ret) {
			fprintf(stderr, "The plugin '%s' has been loaded\n",
					ep->d_name);
		}
	}

	closedir(dp);

	return 0;
}
