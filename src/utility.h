#ifndef UTILITY_H
#define UTILITY_H

#include <vector>

namespace tidalwave{
  /*
   * @brief 便利クラス
   */
  class Utility{
  public:
    /*
     * @brief 指定したディレクトリ配下の全ファイル名をリストで取得する
     * @param dirPath ファイル一覧を取得したいディレクトリのパス
     * @param list 取得したファイル一覧
     */
    static bool getFilesInDirectory(const std::string dirPath, std::vector<std::string> &list, int currentDepth = 1);
  private :
    static const unsigned int SH_PATH_MAX = 4096;
  };
}

#endif // UTILITY_H
