//#include <stdio.h>
//#include "SDL_theora.h"
//int main(int argc, const char* argv[])
//{
//    // Prints each argument on the command line.
//    //for (int i = 0; i < argc; i++)
//    //{
//   
//
//   
//
//    //	printf("arg %d: %s\n", i, argv[i]);
//    //}
//    //While application is running
//    int quit = 0;
//    SDL_Event e;
//    SDL_Window* win = NULL;
//    SDL_Renderer* renderer = NULL;
//    int posX = 100, posY = 100, width = 800, height = 600;
//    win = SDL_CreateWindow("Mercury - NBlood - Theora", posX, posY, width, height, 0);
//    renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
//    int my_video = THR_Load("data/GTI.ogv", renderer);
//
//    SDL_Texture* video_texture = THR_UpdateVideo(my_video);
//
//    while (!quit)
//    {
//        //Handle events on queue
//        while (SDL_PollEvent(&e) != 0)
//        {
//            //User requests quit
//            if (e.type == SDL_QUIT)
//            {
//                quit = 1; //NO ONE WANTS TO PLAY WITH ME!
//            }
//        }
//      
//        // Use SDL_RenderCopy to blit the texture normally
//        
//        //"This promises to be fun!"
//
//        //Clear screen
//        SDL_RenderClear(renderer);
//
//        //Render texture to screen
//        SDL_RenderCopy(renderer, video_texture, NULL, NULL);
//
//        //Update screen
//        SDL_RenderPresent(renderer);
//
//        if (THR_IsPlaying(my_video) == 0)
//        {
//            //SON OF A BITCH MUST PAY!
//            quit = 1;
//            THR_DestroyVideo(my_video);
//        }
//        else
//            video_texture = THR_UpdateVideo(my_video); //I want JoJo! I want JoJo! JOJO! JOJO!
//
//    }
//
//	
//
//
//}