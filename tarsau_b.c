#include "tarsau.h"

bool is_ascii_text_file(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return false;

    int ch;
    while ((ch = fgetc(f)) != EOF) {
        if (ch < 0 || ch > 127) { 
            fclose(f);
            return false;
        }
    }
    fclose(f);
    return true;
}

void handle_archive(int file_count, char *filenames[], const char *output_file) {
    if (file_count > MAX_FILES) {
        printf("Hata: Giriş dosyası sayısı en fazla 32 olabilir!\n");
        exit(0);
    }

    struct stat st;
    long total_size = 0;
    char header_records[8192] = ""; 

    for (int i = 0; i < file_count; i++) {
        if (!is_ascii_text_file(filenames[i])) {
            printf("%s giriş dosyasının formatı uyumsuzdur!\n", filenames[i]);
            exit(0); 
        }

        if (stat(filenames[i], &st) == 0) {
            total_size += st.st_size;
            if (total_size > MAX_TOTAL_SIZE) {
                printf("Hata: Giriş dosyalarının toplam boyutu 200 MB'ı geçemez!\n");
                exit(0);
            }

            char single_record[512];
            sprintf(single_record, "%s,%o,%ld", filenames[i], st.st_mode & 0777, st.st_size);
            strcat(header_records, single_record);

            if (i < file_count - 1) {
                strcat(header_records, " "); 
            }
        } else {
            printf("Hata: %s dosyası okunamadı.\n", filenames[i]);
            exit(0);
        }
    }

    FILE *out_file = fopen(output_file, "wb");
    if (!out_file) {
        printf("Hata: Çıktı dosyası oluşturulamadı!\n");
        exit(0);
    }

    int header_length = strlen(header_records);
    fprintf(out_file, "%010d", header_length);
    fprintf(out_file, "%s", header_records);

    for (int i = 0; i < file_count; i++) {
        FILE *in_file = fopen(filenames[i], "rb");
        if (in_file) {
            int ch;
            while ((ch = fgetc(in_file)) != EOF) {
                fputc(ch, out_file);
            }
            fclose(in_file);
        }
    }

    fclose(out_file);
    printf("Dosyalar birleştirildi.\n");
}
