#include "tonc.h"

#include "text_display.hpp"

#include "pokedex.h"
#include "grayscale.h"

#include "poke.h"

/* Notes about Pokedex
- Mode 1: 2 reg bg, 1 aff
- BG 0: text display
- BG 1: pokedex background
- BG 2: Pokemon Image, half-size affine

*/

void log_pokemon(int num){
	int x = 15;
	int y = 6;
	//load sprite into 
	memcpy16(tile_mem_obj[0], &grayscaleTiles[num*64*8], 16*64);
	TextDisplay::reset();
	TextDisplay::print(x,y++,"(#%%) %%", num+1, poke_data[num].Name);
	y++;
	if(*poke_data[num].Type_2 == '\0'){
		TextDisplay::print(x,y++,"Type: %%", poke_data[num].Type_1);
	}else{
		TextDisplay::print(x,y++,"Type: %%/%%", poke_data[num].Type_1, poke_data[num].Type_2);
	}
	y++;
	TextDisplay::print(x,y++,"HP: %%", poke_data[num].HP);
	TextDisplay::print(x,y++,"Attack: %%", poke_data[num].Attack);
	TextDisplay::print(x,y++,"Defense: %%", poke_data[num].Defense);
	TextDisplay::print(x,y++,"Sp. Attack: %%", poke_data[num].Sp_Attack);
	TextDisplay::print(x,y++,"Sp. Defense: %%", poke_data[num].Sp_Defense);
	TextDisplay::print(x,y++,"Speed: %%", poke_data[num].Speed);
}

OBJ_ATTR curr_pokemon_sprite = {
	ATTR0_BUILD(56,0,0,0,0,0,0),
	ATTR1_BUILDR(24,3,0,0),
	ATTR2_BUILD(0,0,0)
};

int main() {
	irq_init(nullptr);
	irq_add(II_VBLANK, nullptr);

	oam_init(oam_mem, 128);
	oam_copy(oam_mem, &curr_pokemon_sprite, 1);
	memcpy16(pal_obj_mem, grayscalePal, grayscalePalLen/2);

	REG_BG0CNT = BG_BUILD(2,6,0,0,2,0,0);
	LZ77UnCompVram(pokedexTiles, tile_mem[2]);
	LZ77UnCompVram(pokedexMap, se_mem[6]);
	memcpy16(pal_bg_mem, pokedexPal, pokedexPalLen/2);

	REG_BG1CNT = TextDisplay::setCNT(2, 1, 7);

	REG_DISPCNT = DCNT_MODE1 | DCNT_BG0 | DCNT_BG1 | DCNT_OBJ_1D | DCNT_OBJ;

	int curr_poke = 0;
	log_pokemon(curr_poke);
	while(1){
		
		if(key_hit(KEY_RIGHT) || key_hit(KEY_LEFT)){
			curr_poke = clamp(curr_poke + key_tri_horz(), 0, poke_count);
			log_pokemon(curr_poke);
		}else if(key_hit(KEY_UP) || key_hit(KEY_DOWN)){
			curr_poke = clamp(curr_poke + (10*key_tri_vert()), 0, poke_count);
			log_pokemon(curr_poke);
		}

		key_poll();
		VBlankIntrWait();
	};
}
