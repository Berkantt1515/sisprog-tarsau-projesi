#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Hata: Eksik parametre.\n");
        printf("Kullanim: ./tarsau -b [dosyalar] -o [arsiv.sau] VEYA ./tarsau -a [arsiv.sau] [dizin]\n");
        return 1;
    }

    if (strcmp(argv[1], "-b") == 0) {
        int input_file_count = 0;
        char *input_files[32]; 
        char *output_file = "a.sau"; 

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-o") == 0) {
                if (i + 1 < argc) {
                    output_file = argv[i + 1];
                    i++; 
                } else {
                    printf("Hata: -o parametresinden sonra dosya adi belirtilmedi.\n");
                    return 1;
                }
            } else {
                if (input_file_count < 32) {
                    input_files[input_file_count] = argv[i];
                    input_file_count++;
                } else {
                    printf("Hata: En fazla 32 adet giris dosyasi belirtebilirsiniz.\n");
                    return 1;
                }
            }
        }

        long total_size = 0; 
        struct stat file_stat; 

        // --- ORGANIZASYON (HEADER) BILGISINI HAZIRLAMA ---
        char header_records[4096] = ""; // Tum dosya kayitlarini yanyana tutacagimiz metin
        char temp_record[256];          // Tek bir dosyanin kaydini olusturmak icin gecici metin

        for (int i = 0; i < input_file_count; i++) {
            if (stat(input_files[i], &file_stat) == 0) {
                total_size += file_stat.st_size;

                // Format: |Dosya adi, izinler, boyut|
                sprintf(temp_record, "|%s,%o,%ld|", input_files[i], file_stat.st_mode & 0777, file_stat.st_size);
                
                // Olusturulan bu tek kaydi, ana kayit dizgisinin sonuna ekle
                strcat(header_records, temp_record);

            } else {
                printf("HATA: %s dosyasi bulunamadi veya okunamadi!\n", input_files[i]);
                return 1; 
            }
        }

        if (total_size > 200 * 1024 * 1024) {
            printf("HATA: Toplam dosya boyutu 200 MB'i gecemez!\n");
            return 1;
        }

        // Header'in toplam boyutu: Ilk 10 bayt + arkasindan gelen kayitlarin uzunlugu
        int header_total_size = 10 + strlen(header_records);

        // --- ARSIV DOSYASINI OLUSTURMA VE YAZMA ---
        FILE *out_file = fopen(output_file, "w");
        if (out_file == NULL) {
            printf("HATA: %s arsiv dosyasi olusturulamadi!\n", output_file);
            return 1;
        }

        // 1. Organizasyon Bolumunu Yaz (%010d ile tam 10 karakter olacak sekilde sifir dolgulu yazdiriyoruz)
        fprintf(out_file, "%010d%s", header_total_size, header_records);

        // 2. Arsivlenmis Dosyalar (Veri) Bolumunu Yaz
        for (int i = 0; i < input_file_count; i++) {
            FILE *in_file = fopen(input_files[i], "r");
            if (in_file == NULL) {
                printf("HATA: %s dosyasi okunurken hata olustu!\n", input_files[i]);
                fclose(out_file);
                return 1;
            }

            char buffer[1024]; // Veriyi parca parca okumak icin tampon
            size_t bytes_read;
            // Dosyanin sonuna gelene kadar bloklar halinde oku ve oldugu gibi arsive yaz
            while ((bytes_read = fread(buffer, 1, sizeof(buffer), in_file)) > 0) {
                fwrite(buffer, 1, bytes_read, out_file);
            }
            fclose(in_file); // Isimiz biten giris dosyasini kapat
        }

        fclose(out_file); // Arsiv dosyasini tamamen kapat ve kaydet
        printf("%s dosyasi basariyla olusturuldu (Dosyalar Birlestirildi).\n", output_file);

    } 
    else if (strcmp(argv[1], "-a") == 0) {
        printf("Arsivden cikartma (-a) modu secildi. Dosyalar acilacak...\n");
    } 
    else {
        printf("Hatali parametre girdiniz: %s\n", argv[1]);
    }

    return 0;
}
