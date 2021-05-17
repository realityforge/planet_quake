
#include "../Include/RmlUi/Core/Core.h"
#include "../Include/RmlUi/Wrapper.h"
#include "../Include/RmlUi/Core/SystemInterface.h"

extern "C" {

namespace Rml {

  StructuredFileInterface::StructuredFileInterface(RmlFileInterface *file_interface)
  {
    files = file_interface;
  }

  StructuredFileInterface::~StructuredFileInterface()
  {
  }

  Rml::FileHandle StructuredFileInterface::Open(const Rml::String& filepath)
  {
    return files->Open(filepath.c_str());
  }
  
  void StructuredFileInterface::Close(FileHandle file)
  {
    
  }

  size_t StructuredFileInterface::Read(void* buffer, size_t size, FileHandle file)
  {
    return 0;
  }

  bool StructuredFileInterface::Seek(FileHandle file, long offset, int origin)
  {
    return false;
  }

  size_t StructuredFileInterface::Tell(FileHandle file) 
  {
    return 0;
  }

  StructuredSystemInterface::StructuredSystemInterface(RmlSystemInterface *sys)
  {
    system = sys;
  }

  bool StructuredSystemInterface::LogMessage(Log::Type type, const String& message)
  {
    return system->LogMessage(type, message.c_str());
  }

  double StructuredSystemInterface::GetElapsedTime()
  {
    return 0;
  }


  void Rml_SetFileInterface(RmlFileInterface* file_interface) {
    StructuredFileInterface fi = StructuredFileInterface(file_interface);
    Rml::SetFileInterface(fi);
  }

  void Rml_SetRenderInterface(RmlRenderInterface* renderer) {
    Rml::SetRenderInterface(renderer);
  }

  void Rml_SetSystemInterface(RmlSystemInterface* system) {
    StructuredSystemInterface sys = StructuredSystemInterface(system);
    Rml::SetSystemInterface(sys);
  }

  bool Rml_Initialize( void ) {
    return Rml::Initialise();
  }
  
  Context* Rml_CreateContext(const String& name, Vector2i dimensions) {
    Log::Message(Log::LT_INFO, "Made it here!");
    return Rml::CreateContext(name, dimensions);
  }
  
  ElementDocument* Rml_LoadDocument(Rml::Context* ctx, const String& document_path) {
    Log::Message(Log::LT_INFO, "Made it here!");
    return ctx->LoadDocument(document_path);
  }
  
  void Rml_ShowDocument(Rml::ElementDocument* document) {
    //document->Show();
  }
  
  void Rml_Shutdown( void ) {
    Rml::Shutdown();
  }
}

}
