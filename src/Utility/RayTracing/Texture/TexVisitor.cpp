#include <RayTracing/TexVisitor.h>

using namespace CppUtility;
using namespace RayTracing;

void TexVisitor::Visit(const Other::CPtr<ConstTexture> & constTexture) { }
void TexVisitor::Visit(const Other::CPtr<OpTexture> & opTexture) { }
void TexVisitor::Visit(const Other::CPtr<ImgTexture> & imgTexture) { }