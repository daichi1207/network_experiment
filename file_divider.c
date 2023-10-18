#include <stdio.h>

int main() {
  const char *filename = "test.dat";  // バイナリーデータを格納したファイル名

  FILE *file = fopen(filename, "rb");
  if (file == NULL) {
    perror("ファイルのオープンに失敗しました");
    return 1;
  }

  fseek(file, 0, SEEK_END);      // ファイルの末尾に移動
  long file_size = ftell(file);  // ファイルサイズを取得
  rewind(file);                  // ファイルポインタを先頭に戻す

  // ファイルサイズが奇数の場合、分割が均等にならないので注意が必要

  FILE *part1 = fopen("part1.dat", "wb");
  FILE *part2 = fopen("part2.dat", "wb");

  if (part1 == NULL || part2 == NULL) {
    perror("ファイルのオープンに失敗しました");
    return 1;
  }

  // ファイルを2分割する処理
  char buffer;
  for (long i = 0; i < file_size; i++) {
    fread(&buffer, sizeof(char), 1, file);
    if (i < file_size / 2) {
      fwrite(&buffer, sizeof(char), 1, part1);
    } else {
      fwrite(&buffer, sizeof(char), 1, part2);
    }
  }

  fclose(file);
  fclose(part1);
  fclose(part2);

  return 0;
}