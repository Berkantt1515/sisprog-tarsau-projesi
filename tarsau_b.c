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
        // ÖNCE KONTROL: Dosya gerçekten var mı ve okunabiliyor mu?
        if (stat(filenames[i], &st) == 0) {
            
            // DOSYA VARSA İKİNCİ KONTROL: Formatı ASCII mi?
            if (!is_ascii_text_file(filenames[i])) {
                printf("%s giriş dosyasının formatı uyumsuzdur!\n", filenames[i]);
                exit(0); 
            }

            total_size += st.st_size;
            if (total_size > MAX_TOTAL_SIZE) {
                printf("Hata: Giriş dosyalarının toplam boyutu 200 MB'ı geçemez!\n");
                exit(0);
            }

            char single_record[512];
            sprintf(single_record, "|%s,%o,%ld|", filenames[i], st.st_mode & 0777, st.st_size);
            strcat(header_records, single_record);

        } else {
            // DOSYA YOKSA: Doğru hatayı bas!
            printf("Hata: %s dosyası okunamadı veya bulunamadı.\n", filenames[i]);
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

void handle_extract(const char *archive_file, const char *target_dir) {
    FILE *in_file = fopen(archive_file, "rb");
    if (in_file == NULL) {
        printf("Arşiv dosyası uygunsuz veya bozuk!\n");
        exit(0);
    }

    char header_size_str[11];
    if (fread(header_size_str, 1, 10, in_file) != 10) {
        printf("Arşiv dosyası uygunsuz veya bozuk!\n");
        fclose(in_file);
        exit(0);
    }
    header_size_str[10] = '\0';
    int header_total_size = atoi(header_size_str);

    if (header_total_size <= 0) {
         printf("Arşiv dosyası uygunsuz veya bozuk!\n");
         fclose(in_file);
         exit(0);
    }

    char *header_records = malloc(header_total_size + 1);
    if (fread(header_records, 1, header_total_size, in_file) != header_total_size) {
        printf("Arşiv dosyası uygunsuz veya bozuk!\n");
        free(header_records);
        fclose(in_file);
        exit(0);
    }
    header_records[header_total_size] = '\0';

    if (target_dir != NULL) {
        struct stat st = {0};
        if (stat(target_dir, &st) == -1) {
            mkdir(target_dir, 0777); 
        }
    }

    char *token = strtok(header_records, "|");
    char extracted_files[4096] = ""; 
    int extracted_count = 0;

    while (token != NULL) {
        char filename[256];
        unsigned int perms;
        long size;

        if (sscanf(token, "%[^,],%o,%ld", filename, &perms, &size) == 3) {
            char filepath[512];
            if (target_dir != NULL) {
                sprintf(filepath, "%s/%s", target_dir, filename); 
            } else {
                strcpy(filepath, filename); 
            }

            FILE *out_file = fopen(filepath, "wb");
            if (out_file != NULL) {
                char buffer[1024];
                long bytes_left = size;
                
                while (bytes_left > 0) {
                    size_t to_read = (bytes_left < sizeof(buffer)) ? bytes_left : sizeof(buffer);
                    size_t read_now = fread(buffer, 1, to_read, in_file);
                    if (read_now > 0) {
                        fwrite(buffer, 1, read_now, out_file);
                        bytes_left -= read_now;
                    } else {
                        break; 
                    }
                }
                fclose(out_file);
                
                chmod(filepath, perms);

                if (extracted_count > 0) strcat(extracted_files, " ve ");
                strcat(extracted_files, filename);
                extracted_count++;
            }
        }
        token = strtok(NULL, "|"); 
    }

    free(header_records);
    fclose(in_file);
    
    if (target_dir != NULL) {
        printf("%s dizininde %s dosyaları açıldı.\n", target_dir, extracted_files);
    } else {
        printf("Geçerli dizinde %s dosyaları açıldı.\n", extracted_files);
    }
}
