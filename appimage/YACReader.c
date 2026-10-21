#include <limits.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <libgen.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    char tmp[PATH_MAX], exe_base[NAME_MAX + 1], exe_dir[PATH_MAX];
    strncpy(tmp, argv[0], PATH_MAX);
    strncpy(exe_base, basename(tmp), NAME_MAX + 1);
    strncpy(tmp, argv[0], PATH_MAX);
    strncpy(exe_dir, dirname(tmp), PATH_MAX);

    const char *appimage_path = getenv("APPIMAGE");
    if (appimage_path && *appimage_path) {
        char ai_base[NAME_MAX + 1], ai_dir[PATH_MAX];
        strncpy(tmp, appimage_path, PATH_MAX);
        strncpy(ai_base, basename(tmp), NAME_MAX + 1);
        strncpy(tmp, appimage_path, PATH_MAX);
        strncpy(ai_dir, dirname(tmp), PATH_MAX);

        char *ai_last_part = strrchr(ai_base, '-');
        if (!ai_last_part || !*ai_last_part)
            ai_last_part = strrchr(ai_base, '.');
        if (ai_last_part && *ai_last_part) {
            snprintf(tmp, PATH_MAX, "%s/%s%s", ai_dir, exe_base, ai_last_part);
            argv[0] = tmp;
            execvp(argv[0], argv);
            if (errno != ENOENT)
                return 1;
            argv[0] = basename(tmp);
            execvp(argv[0], argv);
            if (errno != ENOENT)
                return 1;
        }
        snprintf(tmp, PATH_MAX, "%s/%s", ai_dir, exe_base);
        argv[0] = tmp;
        execvp(argv[0], argv);
        if (errno != ENOENT)
            return 1;
    }

    const char *env_path = getenv("PATH");
    if (!env_path || !*env_path)
        return 1;

    char exe_dir_rp[PATH_MAX];
    if (!realpath(exe_dir, exe_dir_rp))
        return 1;

    char env_path_cp[PATH_MAX];
    strncpy(env_path_cp, env_path, PATH_MAX);

    char ep_rp[PATH_MAX];
    for (char *p = strtok(env_path_cp, ":"); p; p = strtok(NULL, ":")) {
        if (!realpath(p, ep_rp))
            continue;
        if (strcmp(exe_dir_rp, ep_rp)) {
            snprintf(tmp, PATH_MAX, "%s/%s", ep_rp, exe_base);
            argv[0] = tmp;
            execvp(argv[0], argv);
            if (errno != ENOENT)
                return 1;
        }
    }
    return 1;
}
