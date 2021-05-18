
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
    if(system->LogMessage)
      return system->LogMessage(type, message.c_str());
    return false;
  }

  double StructuredSystemInterface::GetElapsedTime()
  {
    return 0;
  }

  StructuredRenderInterface::StructuredRenderInterface(RmlRenderInterface *rend)
  {
    renderer = rend;
  }
  
  void StructuredRenderInterface::RenderGeometry(Vertex* vertices, int num_vertices, int* indices, int num_indices, TextureHandle texture, const Vector2f& translation) {
    
  }
  void StructuredRenderInterface::EnableScissorRegion(bool enable) {
    
  }
  void StructuredRenderInterface::SetScissorRegion(int x, int y, int width, int height) {
    
  }


  void Rml_SetFileInterface(RmlFileInterface* file_interface) {
    static StructuredFileInterface fi(file_interface);
    Rml::SetFileInterface(&fi);
  }

  void Rml_SetRenderInterface(RmlRenderInterface* renderer) {
    static StructuredRenderInterface rend(renderer);
    Rml::SetRenderInterface(&rend);
  }

  void Rml_SetSystemInterface(RmlSystemInterface* system) {
    static StructuredSystemInterface sys(system);
    Rml::SetSystemInterface(&sys);
  }

  bool Rml_Initialize( void ) {
    return Rml::Initialise();
  }
  
  Context* Rml_CreateContext(const String& name, Vector2i dimensions) {
    return Rml::CreateContext(name, dimensions);
  }
  
  ElementDocument* Rml_LoadDocument(Rml::Context* ctx, const char *document_path) {
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
