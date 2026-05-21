#include "tarsau.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Kullanım: tarsau -b <dosyalar> -o <cikti_dosyasi>\nVEYA tarsau -a <arsiv_dosyasi> [hedef_dizin]\n");
        return 0;
    }

    if (strcmp(argv[1], "-b") == 0) {
        char *input_files[MAX_FILES];
        int input_file_count = 0;
        const char *output_filename = "a.sau"; 

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-o") == 0) {
                if (i + 1 < argc) {
                    output_filename = argv[i + 1];
                    i++; 
                }
            } else {
                if (input_file_count < MAX_FILES) {
                    input_files[input_file_count++] = argv[i];
                } else {
                    // YENİ EKLENEN KONTROL: 32 sınırı aşılırsa işlemi anında kes!
                    printf("Hata: Giriş dosyası sayısı en fazla 32 olabilir!\n");
                    return 0;
                }
            }
        }

        if (input_file_count == 0) {
            printf("Hata: Giriş dosyası belirtilmedi!\n");
            return 0;
        }

        handle_archive(input_file_count, input_files, output_filename);
    } 
    else if (strcmp(argv[1], "-a") == 0) {
        if (argc < 3) {
            printf("Hata: -a parametresinden sonra arşiv dosyası belirtilmelidir.\n");
            return 1;
        }

        const char *archive_file = argv[2];
        const char *target_dir = NULL;

        if (argc >= 4) {
            target_dir = argv[3];
        }

        handle_extract(archive_file, target_dir);
    }

    return 0;
}
