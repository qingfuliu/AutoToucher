#include <fstream>
#include "configurator/auto_config.h"
auto main() -> int {
  //  auto v = json::Json();
  //  v.ParserFromStream(s);
  //  std::cout << v["a"].IsInteger() << "  \n" << v["b"].IsFloat() << "  \n" << v["c"].IsArray() << "  \n";
  //  v["k"] = 1;
  //  v.SerializeToStream(std::cout);
  //  return 0;
  std::fstream s;
  s.open("/home/lqf/WorkStation/AutoToucher/test.json");
  for (int i = 0; i <= 32; ++i) {
    s << "#define DESERIALIZE_HELPER_" + std::to_string(i) + "(XX, CTX, X1, X2, ...)                       \\"
      << std::endl;
    s << "XX(CTX, X1, X2)                                                      \\" << std::endl;
    s << "HELPER_CONCAT(HELPER_INVOKE_IF_NOT_EMPTY_, HELPER_NARG(__VA_ARGS__)) \\" << std::endl;
    s << "(DESERIALIZE_HELPER_" + std::to_string(i + 1) + ", XX, CTX, __VA_ARGS__)" << std::endl << std::endl;
  }
  return 1;
}