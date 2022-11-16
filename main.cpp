
#include "window.h"
#include "bitmap.h"


window app;

int main(){

    bitmap loader;

    const char *FILENAME= "text.bmp";
//        const char *FILENAME= "aloe_bee.bmp";

    if(loader.LoadFile(FILENAME)){

        if(app.Initialize(&loader)){

            int rt= app.Start();

            if(rt!= -1){
                loader.WriteFile("current.bmp");
            }/* write currently loaded bitmap if no errors*/

            return rt;
        }

    }else { printf("bitmap error \n"); }

}
