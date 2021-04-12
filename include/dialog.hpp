#include <citro3d.h>
#include <vector>
#include <string>

class Dialog {
    private:
    std::vector<std::string> options;
    std::vector<std::string> message;
    C3D_RenderTarget* target;
    float x,y,width,height;
    int selected;
    void draw();
    public:
    int handle();
    Dialog(C3D_RenderTarget* target, float x, float y, float width, float height, std::string message, std::initializer_list<std::string> options, int defaultChoice=0);
    Dialog(C3D_RenderTarget* target, float x, float y, float width, float height, std::initializer_list<std::string> message, std::initializer_list<std::string> options, int defaultChoice=0);
    
};