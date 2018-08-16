#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <memory.h>

#include <getopt.h>
#include <sqlite3.h>

const char suffix[] = ".bak";
int main(int argc, char** argv) {
    int timeout = 5000;
    int pages = 5;
    int sleep = 250;
    char* output = NULL;

    while (1) {
		static struct option long_options[]={
			{ "timeout",  required_argument, 0, 't' },
            { "pages",    required_argument, 0, 'p' },
            { "sleep",    required_argument, 0, 's' },
            { "output",   required_argument, 0, 'o' },
			{ 0, 0, 0, 0 }
		};
		int option_index = 0;
		int c = getopt_long(argc, argv, "t:p:s:o:", long_options, &option_index);
		if (c == -1) {
			break;
		}
		switch(c) {
			case 'w': timeout  = strtol(optarg, NULL, 10); break;
            case 'p': pages = strtol(optarg, NULL, 10); break;
            case 'o': output = strdup(optarg); break;
		}
	}

    if (timeout <= 0) {
        puts("Timeout must be a positive integer.");
        return 1;
    }

    if (pages <= 0) {
        puts("Pages must be a positive integer.");
        return 1;
    }

    if (sleep <= 0) {
        puts("Sleep must be a positive integer.");
        return 1;
    }

    if (optind >= argc) {
		puts("Expected a database filename.");
		return 1;
	}

    char* filename = argv[optind];

    // output to <dbfilename>.bak
    if (output == NULL) {
        size_t slen = strlen(filename);
        output = malloc(slen + sizeof(suffix) + 1);
        strcpy(output, filename);
        strcat(output, suffix);
    }

    sqlite3* srcdb = NULL;
    sqlite3* dstdb = NULL;
    sqlite3_backup *sbk = NULL;

    int rc = 0, rv = 0;

    rc = sqlite3_open(filename, &srcdb);
    if (rc != SQLITE_OK) {
        rv = 1;
        goto cleanup;
    }

    rc = sqlite3_open_v2(output, &dstdb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (rc != SQLITE_OK) {
        rv = 1;
        goto cleanup;
    }

    sqlite3_busy_timeout(srcdb, sleep);
    sqlite3_busy_timeout(dstdb, sleep);

    sbk = sqlite3_backup_init(dstdb, "main", srcdb, "main");
    if (sbk == NULL) {
        goto errormsg;
    }

    do {
        rc = sqlite3_backup_step(sbk, pages);
        if (rc != SQLITE_DONE && rc != SQLITE_OK) {
            goto errormsg;
        }
    } while(rc != SQLITE_DONE);

    goto cleanup;
errormsg:
    rv = 2;
    puts(sqlite3_errmsg(dstdb));
cleanup:
    if (sbk != NULL)
        sqlite3_backup_finish(sbk);
    if (srcdb != NULL)
        sqlite3_close(srcdb);
    if (dstdb != NULL)
        sqlite3_close(dstdb);
    free(output);
    return rv;
}