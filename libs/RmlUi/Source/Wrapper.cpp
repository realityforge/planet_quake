
#include "../Include/RmlUi/Core/Core.h"
#include "../Include/RmlUi/Wrapper.h"
#include "../Include/RmlUi/Core/SystemInterface.h"
#include "../Include/RmlUi/Core/ElementDocument.h"

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
    return files->Close(file);
  }

  size_t StructuredFileInterface::Read(void* buffer, size_t size, FileHandle file)
  {
    return files->Read(buffer, size, file);
  }

  bool StructuredFileInterface::Seek(FileHandle file, long offset, int origin)
  {
    return files->Seek(file, offset, origin);
  }

  size_t StructuredFileInterface::Tell(FileHandle file) 
  {
    return files->Tell(file);
  }

  size_t StructuredFileInterface::Length(FileHandle file) 
  {
    return files->Length(file);
  }

  bool StructuredFileInterface::LoadFile(const String& path, String& out_data) 
  {
    char *data;
    int len = files->LoadFile(path.c_str(), &data);
    out_data = std::string(data, len);
    return len > -1;
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
    if(renderer->RenderGeometry) {
      float trans[2];
      trans[0] = translation.x;
      trans[1] = translation.y;
      //printf("Render1: %f x %f\n", vertices[0].position[0], vertices[0].position[1]);
      //printf("Render2: %f x %f\n", vertices[1].position[0], vertices[1].position[1]);
      //printf("Render3: %f x %f\n", vertices[2].position[0], vertices[2].position[1]);
      //printf("Render4: %f x %f\n", vertices[3].position[0], vertices[3].position[1]);
      renderer->RenderGeometry(vertices, num_vertices, indices, num_indices, texture, trans);
    }
  }
  void StructuredRenderInterface::EnableScissorRegion(bool enable) {
    //printf("Render:\n");
  }

  void StructuredRenderInterface::SetScissorRegion(int x, int y, int width, int height) {
    //printf("Render:\n");
  }

  bool StructuredRenderInterface::GenerateTexture(TextureHandle& texture_handle, const byte* source, const Vector2i& source_dimensions) {
    texture_handle = renderer->GenerateTexture(source, source_dimensions);
    return texture_handle > 0;
  }

  bool StructuredRenderInterface::LoadTexture(TextureHandle& texture_handle, Vector2i& texture_dimensions, const String& source) {
    int dimen[2];
    texture_handle = renderer->LoadTexture(dimen, source.c_str());
    texture_dimensions.x = dimen[0];
    texture_dimensions.y = dimen[1];    
    return texture_handle > 0;
  }

  Q_EXPORT void Rml_SetFileInterface(RmlFileInterface* file_interface) {
    static StructuredFileInterface fi(file_interface);
    Rml::SetFileInterface(&fi);
  }

  Q_EXPORT void Rml_SetRenderInterface(RmlRenderInterface* renderer) {
    static StructuredRenderInterface rend(renderer);
    Rml::SetRenderInterface(&rend);
  }

  Q_EXPORT void Rml_SetSystemInterface(RmlSystemInterface* system) {
    static StructuredSystemInterface sys(system);
    Rml::SetSystemInterface(&sys);
  }

  Q_EXPORT void Rml_LoadFontFace(const char *name, qboolean fallback) {
    Rml::LoadFontFace(name, fallback);
  }


  Q_EXPORT bool Rml_Initialize( void ) {
    return Rml::Initialise();
  }
  
  static Context *ctx;
  Q_EXPORT int Rml_CreateContext(const char *name, int width, int height) {
    ctx = Rml::CreateContext(name, Rml::Vector2i(width, height));
    return ctx != nullptr ? 1 : 0;
  }
  
  static ElementDocument *doc;
  Q_EXPORT int Rml_LoadDocument(int _, const char *document_path) {
    doc = ctx->LoadDocument(document_path);
    return doc != nullptr ? 1 : 0;
  }
  
  Q_EXPORT void Rml_ShowDocument(int document) {
    doc->Show();
  }
  
  Q_EXPORT void Rml_ContextRender( qhandle_t _ ) {
    ctx->Render();
  }

  Q_EXPORT void Rml_ContextUpdate( qhandle_t _ ) {
    ctx->Update();
  }

  Q_EXPORT void Rml_Shutdown( void ) {
    Rml::Shutdown();
  }
}

}
