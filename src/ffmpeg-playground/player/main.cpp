#include "stdafx.h"

#ifdef main
#undef main
#endif

int main(int argc, const char *argv[]) 
{
	// if(argc != 2){
	// 	std::cout << "Usage: ./player <video_addr>"<<std::endl;
	// 	exit(-1);
	// }

	SDLWrapper::init_sdl();

	Player::get_instance()->run("test", "VideoPlayder");

	Player::get_instance()->clear();
}