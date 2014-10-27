#include "producer.h"
#include "utility.h"

using namespace std;

namespace tidalwave {

  Producer::Producer(MessageQueue<Request> &reqq)
      : requestQueue(reqq) {
  }

  int Producer::run(const Parameter &param) {

    vector<string> expect_files;
    vector<string> target_files;

    Utility::getFilesInDirectory(param.expect_path, expect_files);
    Utility::getFilesInDirectory(param.target_path, target_files);

    int request_count = 0;

    for (vector<string>::iterator target_it = target_files.begin(); target_it != target_files.end(); ++target_it) {
      for (vector<string>::iterator expect_it = expect_files.begin(); expect_it != expect_files.end(); ++expect_it) {
        string expect_image = (*expect_it).substr(param.expect_path.length() + 1);
        string target_image = (*target_it).substr(param.target_path.length() + 1);
        // expect_pathとtarget_pathの中に同じ名前のファイルが見つかったら処理を依頼
        if (target_image == expect_image) {
          Request req;
          req.expect_image = *expect_it;
          req.target_image = *target_it;
          std::cout << "request: " << expect_image << " <-> " << target_image << endl;
          requestQueue.push(req);
          request_count++;
          break;
        }
      }
    }

    cout << "finish producer" << endl;
    return request_count;
  }
}
