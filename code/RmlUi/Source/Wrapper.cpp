
#include "../Include/RmlUi/Core/Core.h"

extern "C" {

namespace Rml {

  void Rml_SetFileInterface(FileInterface* file_interface) {
    Rml::SetFileInterface(file_interface);
  }
}

}
