#include "tonc.h"

#include "terminal.hpp"

#include "image.h"

#include "example.h"

int main(){
    //enable Border BG
    u8 cbb = 0;
    u8 sbb = 16;
    REG_BG0CNT = BG_BUILD(cbb, sbb, 0, 0, 1, 0, 0);

    //load palette
    memcpy16(pal_bg_mem, imagePal, imagePalLen/2);

    //load tiles
    LZ77UnCompVram(imageTiles, tile_mem[cbb]);
    
    //load image
    memcpy16(&se_mem[sbb], imageMap, imageMapLen/2);

    //enable Text BG
    REG_BG1CNT = Terminal::setCNT(1, cbb+1, sbb+1);
    REG_DISPCNT = DCNT_BG0 | DCNT_BG1 | DCNT_MODE0;

    Terminal::log("%% Initial. %%: # from %%",example_headers[0], example_headers[1], example_headers[2]);
    for(int i = 0; i < example_count; i++){
        Terminal::log("%% %%. Age: %% from %%", example_data[i].Name, example_data[i].Middle_Initial, example_data[i].Age, example_data[i].City);
    }
    while(1){}
}