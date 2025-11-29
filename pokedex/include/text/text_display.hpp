#ifndef __TEXT_DISPLAY__
#define __TEXT_DISPLAY__

#include "tonc.h"

#include "font.hpp"

class TextDisplay{
    public:
        static u16 setCNT(u8 bg, u16 cbb, u16 sbb);

        static void reset();

        static void print(int xCol, int yRow, const char* word){
            curr_line_num = yRow;
            curr_tile_num = xCol;
            if(need_clear_line){
                advanceOneLine();
                need_clear_line = false;
            }
            while(*word != '\0'){
                //draw next char
                drawVal(*word);
                word++;
            }
            need_clear_line = true;
            curr_pixel_num = 0;
        };


        template <typename T, typename... Args>
        static void print(int xCol, int yRow, const char* word, T val, Args... args){
            curr_line_num = yRow;
            curr_tile_num = xCol;
            if(need_clear_line){
                advanceOneLine();
                need_clear_line = false;
            }
            while(*word != '\0'){

                //'%%' will be used to display a char or int
                if(*word == '%' && *word == '%'){
                    //draw value
                    drawVal(val);
                    //move past '%%'
                    word += 2;
                    //continue logging w/o val
                    print(curr_tile_num, curr_line_num, word, args...);
                    return;
                }else{
                    //draw next char
                    drawVal(*word);
                    word++; 
                }
            }
            need_clear_line = true;
            curr_pixel_num = 0;
        };


    private:
        static void loadTile(char c, int vram_tile_ind);
        static void drawVal(int val);
        static void drawVal(char c);
        static void drawVal(const char* c);
        static void clearTopRow();
        static void updateScreen();
        static void advanceOneLine();

        static char intToChar(int x);
        static u32 tileRowShiftR(u32 val, u8 rhs);
        static u32 tileRowShiftL(u32 val, u8 lhs);
        
        static u16 curr_line_num;
        static u16 curr_tile_num;
        static u8 curr_pixel_num;
        static bool need_clear_line;

        //For DCNT_BG$(0-3)
        static u8 bg_ind;

        static u16 text_sbb;
        static u16 text_cbb;
        static font text_font;
};

#endif