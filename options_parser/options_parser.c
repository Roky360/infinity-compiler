#include "options_parser.h"
#include "../config/console_colors.h"
#include "../config/globals.h"
#include "../logging/logging.h"
#include "../io/io.h"
#include <stdio.h>
#include <unistd.h>

int parse_options(int argc, char **argv) {
    int c, next_arg;
    while ((c = getopt(argc, argv, "hc:")) != -1) {
        switch (c) {
            case 'h': // help
                op_help();
                next_arg = -1;
                break;
            case 'c': // color
                op_color(optarg);
                next_arg = optind;
                break;
            case 'd': // debug flag
                op_debug(optarg);
                next_arg = optind;
                break;
            case '?':
                if (optopt == 'c' || optopt == 'd')
                    fprintf(stderr, "Option -%c requires an argument (on/off).\n", optopt);
                next_arg = -1;
                break;
            default:
                fprintf(stderr, "Unknown option -%c. Type -h for all the options.\n", optopt);
                next_arg = -1;
                break;
        }
    }
    return next_arg;
}

void set_option(char op, int mode) {
    FILE *fp;
    int x;
    char *op_ptr, *content = read_file(OPTIONS_FILE);
    if (content) {
        op_ptr = strchr(content, op) + 2; // skip the character and the '=' (c=1)
        *op_ptr = (char) (mode + '0');
        fp = fopen(OPTIONS_FILE, "w");
        if (fp) {
            write_to_file(fp, content);
        }
    }
}

void op_debug(const char *mode) {
    if (strcasecmp(mode, "on") == 0) {
        set_option('d', 1);
#if 1
#define INF_DEBUG
#endif
        log_info(COMPILER, "Debug mode off");
    } else if (strcasecmp(mode, "off") == 0) {
        set_option('d', 0);
#if 1
#undef INF_DEBUG
#endif
        log_info(COMPILER, "Debug mode on");
    } else {
        printf("-d option needs to be followed by \"on\" or \"off\". Ignoring option.");
        return;
    }
}

void op_color(const char *mode) {
    int flag;
    if (strcasecmp(mode, "on") == 0) {
        set_option('c', 1);
        flag = 1;
#if flag == 1
#define INF_SHOW_COLORS
#endif
        log_info(COMPILER,
                 RED "c" YELLOW "o" YELLOW_H "l" GREEN "o" BLUE "r" CYAN "s " CYAN_H "o" MAGENTA "n" MAGENTA_H "!");
    } else if (strcasecmp(mode, "off") == 0) {
        set_option('c', 0);
        flag = 0;
#if flag == 0
#undef INF_SHOW_COLORS
#endif
        log_info(COMPILER, "colors off");
    } else {
        printf("-d option needs to be followed by \"on\" or \"off\". Ignoring option.");
        return;
    }
}

void op_help() {
    const char *op_color = MAGENTA_BH;
#ifdef INF_SHOW_COLORS
    printf(CYAN_BH);
#endif
    printf("Infinity Compiler Help:\n");
    // -c color
#ifdef INF_SHOW_COLORS
    printf("%s", op_color);
#endif
    printf("-c\t");
#ifdef INF_SHOW_COLORS
    printf(RESET);
#endif
    printf("Use colors when printing. Type \"on\" or \"off\" after this option\n");
    // -d debug
#ifdef INF_SHOW_COLORS
    printf("%s", op_color);
#endif
    printf("-d\t");
#ifdef INF_SHOW_COLORS
    printf(RESET);
#endif
    printf("Set the debugging flag on or off. Type \"on\" or \"off\" after this option\n");
    // -h help
#ifdef INF_SHOW_COLORS
    printf("%s", op_color);
#endif
    printf("-h\t");
#ifdef INF_SHOW_COLORS
    printf(RESET);
#endif
    printf("Show this menu\n");

    printf("Options need to be specified before input and output paths.\n\n");
//    printf(
//            CYAN_BH "Infinity Compiler Help:\n"
//            MAGENTA_BH "-c\t" RESET "Use colors when printing. Type \"on\" or \"off\" after this option\n"
//            MAGENTA_BH "-d\t" RESET "Set the debugging flag on or off. Type \"on\" or \"off\" after this option\n"
//            MAGENTA_BH "-h\t" RESET "Show this menu\n"
//    );
}
