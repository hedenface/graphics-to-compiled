#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_INFO_LINE_LEN 255

#define FALSE 0
#define TRUE 1

#define EXIT_WRONG_NO_OF_ARGS 1
#define EXIT_NO_FILE 2
#define EXIT_NO_MEMORY_LEFT 3

#define RED 0
#define GREEN 1
#define BLUE 2

#define NO_MEMORY_MSG "Out of memory. Larger problems coming in 5.. 4.. 3.."

#define check_null(ptr) \
    do { \
        if (ptr == NULL) { \
            printf("%s\n", NO_MEMORY_MSG); \
            exit(EXIT_NO_MEMORY_LEFT); \
        } \
    } while (0)

#define _calloc(ptr, nmemb, size) \
    do { \
        ptr = calloc(nmemb, size); \
        if (ptr == NULL) { \
            printf("%s\n", NO_MEMORY_MSG); \
            exit(EXIT_NO_MEMORY_LEFT); \
        } \
    } while (0)

#define ltrim(str) \
        do { \
            while (isspace(*str)) { \
                str++; \
            } \
        } while (0)

#define rtrim(str) \
        do { \
            int rtrim_i = strlen(str); \
            for (; rtrim_i > 0; rtrim_i--) { \
                if (!isspace(str[rtrim_i - 1])) { \
                    break; \
                } \
                str[rtrim_i - 1] = '\0'; \
            } \
        } while (0)

#define trim(str) \
    do { \
        ltrim(str); \
        rtrim(str); \
    } while(0)

#define str_eq(str, cmp) strcasecmp(str, cmp) == 0
#define key_eq(cmp) str_eq(key, cmp)
#define value_eq(cmp) str_eq(value, cmp)

#define str_starts_with(haystack, needle) (haystack == strstr(haystack, needle))
#define key_starts_with(needle) str_starts_with(key, needle)
#define value_starts_with(needle) str_starts_with(value, needle)

#define ishexstr(str) (str_starts_with(str, "0x") || str_starts_with(str, "0X"))
#define isbinstr(str) (str_starts_with(str, "0b") || str_starts_with(str, "0B"))

typedef struct resource_info_t {
    unsigned int width;
    unsigned int height;
    int transparency;
    unsigned char transparency_rgb[3];
    void * data;
    char * tab;
    char * array_type;
    char * array_name;
    int bytes_per_line;
} resource_info;

int file_exists(char * file)
{
    FILE * fp = fopen(file, "r");

    if (fp != NULL) {
        fclose(fp);
        return TRUE;
    }

    return FALSE;
}

void usage(const char * arg0)
{
    printf("%s\n\n", "Usage:");
    printf("%s [/path/to/base]\n", arg0);
    printf("%s", "\n");
    printf("%s\n\n", "Application will expect /path/to/base.info and /path/to/base.bmp");
    printf("%s\n", "Contents of /path/to/base.info:");
    printf("%s\n", "transparency = true");
    printf("%s\n", "transparency_red = 155");
    printf("%s\n", "transparency_green = 155");
    printf("%s\n", "transparency_blue = 155");
    printf("%s\n", "");
}

void check_args(int argc, const char * arg0)
{
    if (argc < 2) {
        usage(arg0);
        exit(EXIT_WRONG_NO_OF_ARGS);
    }    
}

char * check_file(const char * base, const char * extension)
{
    char * file = NULL;
    size_t file_len = strlen(base) + strlen(extension) + 1;

    _calloc(file, file_len, sizeof(char));
    snprintf(file, file_len, "%s%s", base, extension);

    if (file_exists(file) == FALSE) {
        printf("\n\n%s\n", "ERROR:");
        printf("  Unable to locate file '%s'!\n\n", file);
        free(file);
        exit(EXIT_NO_FILE);
    }

    return file;
}

void parse_info_line(resource_info * info, char * line)
{
    char * key = strtok(line, "=");
    char * value = strtok(NULL, "\n");
    int num_value = 0;

    if (key == NULL) {
        return;
    }
    if (value == NULL) {
        return;
    }

    trim(key);
    trim(value);

    if (key_eq("") || value_eq("")) {
        return;
    }

    if (key_eq("transparency")) {
        if (value_eq("true") || atoi(value)) {
            info->transparency = TRUE;
            return;
        }
    }
    if (key_starts_with("transparency_")) {
        if (ishexstr(value)) {
            num_value = strtol(value, NULL, 16);
        }
        else if (isbinstr(value)) {
            num_value = strtoul(value + 2, NULL, 2);
        }
        else {
            num_value = atoi(value);
        }

        if (key_eq("transparency_red")) {
            info->transparency_rgb[RED] = num_value;
        }
        if (key_eq("transparency_green")) {
            info->transparency_rgb[GREEN] = num_value;
        }
        if (key_eq("transparency_blue")) {
            info->transparency_rgb[BLUE] = num_value;
        }

        return;
    }

    if (key_eq("type") || key_eq("array_type")) {
        free(info->array_type);
        info->array_type = strdup(value);
        return;
    }

    if (key_eq("name") || key_eq("array_name")) {
        free(info->array_name);
        info->array_name = strdup(value);
        return;
    }

    if (key_eq("bytes") || key_eq("bytes_per_line")) {
        info->bytes_per_line = atoi(value);
        if (info->bytes_per_line < 1) {
            info->bytes_per_line = 1;
        }
        return;
    }

    if (key_eq("tab")) {
        /* this is whacky, but we don't really care what it's wrapped in
           after it gets trimmed. take a value of `"   "` - this needs
           to be three spaces. but we don't care if it's `[   ]` or `'   '`
           either. so we increment the start by 1, and null term length - 1
           to speed things up (again, because we don't care */
        size_t len = strlen(value);
        value[len - 1] = '\0';
        value++;
        free(info->tab);
        info->tab = strdup(value);
        return;
    }
}

void parse_info(resource_info * info, char * file_info, char * file_bmp)
{
    FILE * fp = fopen(file_bmp, "rb");
    unsigned char bmp_header[54];
    char line[MAX_INFO_LINE_LEN];

    fread(bmp_header, sizeof(unsigned char), 54, fp);
    fclose(fp);

    info->width = bmp_header[18];
    info->height = bmp_header[22];

    fp = fopen(file_info, "r");
    while (fgets(line, MAX_INFO_LINE_LEN - 1, fp)) {
        parse_info_line(info, line);
    }

    fclose(fp);
}

unsigned char * parse_bmp(resource_info * info, char * file_bmp)
{
    FILE * fp = fopen(file_bmp, "rb");
    unsigned char bmp_header[54];
    unsigned char * file_data = NULL;
    unsigned char * data = NULL;
    int i = 0;
    int j = 0;

    /* we've already pulled this info, this is just to disgard the offset */
    fread(bmp_header, sizeof(unsigned char), 54, fp);

    _calloc(file_data, info->width * info->height * 3, sizeof(unsigned char));
    _calloc(data, info->width * info->height * 4, sizeof(unsigned char));

    fread(file_data, sizeof(unsigned char), info->width * info->height * 3, fp);
    fclose(fp);

    for (i = 0; i < info->width * info->height * 3; i++) {

        data[j++] = file_data[i];

        /* we need to transform from 3 bytes to 4 bytes
           to add transparency (even if its disabled) */
        if ((i + 1) % 3 == 0) {

            /* if the last pixel RGB values match what we're looking
               for with transparency, then mark it as such - otherwise
               mark it as opaque */
            if (info->transparency == TRUE
                    && data[j - 3] == info->transparency_rgb[RED]
                    && data[j - 2] == info->transparency_rgb[GREEN] 
                    && data[j - 1] == info->transparency_rgb[BLUE]) {
                data[j++] = 0;
            } else {
                data[j++] = 255;
            }
        }
    }

    free(file_data);
    return data;
}

void print_resource(resource_info * info, unsigned char * data)
{
    int i = 0;

    printf("%s %s[%d] = {\n", info->array_type, info->array_name, info->width * info->height * 4);

    printf("%s", info->tab);
    for (i = 0; i < info->width * info->height * 4; i++) {
        printf("0x%02x", data[i]);

        if (i < (info->width * info->height * 4) - 1) {
            printf("%s", ", ");

            if ((i + 1) % info->bytes_per_line == 0) {
                printf("%s%s", "\n", info->tab);
            }
        }
    }

    printf("\n%s\n", "};");
}

void set_resource_defaults(resource_info * info)
{
    info->tab = strdup("    ");
    info->array_type = strdup("unsigned char");
    info->array_name = strdup("array");
    info->bytes_per_line = 16;
}

int main(int argc, char const *argv[])
{
    resource_info * info = NULL;
    unsigned char * data = NULL;
    char * file_bmp = NULL;
    char * file_info = NULL;

    _calloc(info, 1, sizeof(resource_info));
    set_resource_defaults(info);

    check_args(argc, argv[0]);
    file_bmp = check_file(argv[1], ".bmp");
    file_info = check_file(argv[1], ".info");

    parse_info(info, file_info, file_bmp);
    data = parse_bmp(info, file_bmp);

    print_resource(info, data);

    free(data);
    free(info->tab);
    free(info->array_name);
    free(info->array_type);
    free(info);
    free(file_bmp);
    free(file_info);
    return 0;
}
