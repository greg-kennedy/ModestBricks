/* Modest Bricks by Greg Kennedy
  Feb. 2 2005
   Yet another Tetris clone.  This is my first real attempt at using
   SDL.  Pardon the messy code layout, I really have no idea where to
   start with encapsulation.  Oh well.  */
   
/* I took the advice of a guide I saw and went all out on this one.  Scoring,
  high score, levels and speed increases, lookahead, custom sound effect and music,
  game options... the whole works.  The code is a total wreck and I'm glad not to
  have to look at it again (except maybe trying to do Linux port work) but maybe,
  just maybe, you could glean something from it.  Good luck. */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

#include <ctype.h>
#include <time.h>

#include <SDL/SDL.h>


long getMS()
{
	return SDL_GetTicks();
}

int main()
{
	int kD=0;

    Mix_Music *music = NULL;   /* SDL_Mixer stuff here */
    Mix_Chunk *crash = NULL;

	SDL_Surface *screen;	/*main screen, temp bitmap, seven block images */
	SDL_Surface *bitmap = NULL;    /* also a surface for the numbers */
	SDL_Surface *bimg[7];
	SDL_Surface *numbers;

	SDL_Rect src,dst,play,quit,mus,sfx;		/*temp rectangles, four block pieces */
	SDL_Rect block[4];

	int i, j, sfxon=1,muson=1,tetri,playing=0,offl=0,offt=0,nextmove=0,cur=0,lookahead,menuevent=0,level=0,lines=0,linectr,p,q,paused=0;
	long starting;
	long score,hiscore,tscore;     /* I sure do use a lot of variables... */
	SDL_Event event;
	FILE* hifile;

//	int crashChannel = -1;


  int audio_rate = 44100;
  Uint16 audio_format = AUDIO_S16; /* 16-bit stereo */
  int audio_channels = 2;
  int audio_buffers = 4096;

	typedef struct {       /* A structure to hold piece information. */
		int b1x;
		int b1y;
		int b2x;
		int b2y;
		int b3x;
		int b3y;
		int b4x;
		int b4y;
	} piecestruct;

	piecestruct piece;        /* we need one for current and one for next piece */
	piecestruct nextpiece;
	
	char tempstring[20];

	char board[10][21] ={{0}};		/* board positions */
	char bmpname[18]="tetimg/0brick.bmp";

	/* Initialize SDL */
	if ( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0 ) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n",SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);
	
	/* I want to load the first block image and then make that my window icon. */
    SDL_WM_SetIcon(SDL_LoadBMP("tetimg/1brick.bmp"), NULL);
 
    /* I want a 380x504 window, double buffered, any format (16/32 bit color) */
	screen=SDL_SetVideoMode(380,504,0,SDL_ANYFORMAT | SDL_DOUBLEBUF);
	srand((unsigned)time(NULL));

/* This is where we open up our audio device.  Mix_OpenAudio takes
     as its parameters the audio format we'd /like/ to have. */
  if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)) {
    printf("Unable to open audio!\n");
    exit(1);
  }

  /* If we actually care about what we got, we can ask here.  In this
     program we don't, but I'm showing the function call here anyway
     in case we'd want to know later. */
  Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);

  /* This is loading a sound effect for a block landing. */
  crash = Mix_LoadWAV("fall.wav");

  /* This is the music to play. */
	music = Mix_LoadMUS("modest.mid");



	for(i=0;i<7;i++) {	/* load the other six block bitmaps */
		bmpname[7]=toascii(i+49);
		bimg[i]=SDL_LoadBMP(bmpname);
		if ( bimg[i]  == NULL ) {
			printf("Unable to load %s.\n",bmpname);
			exit(1);
		}
	}
	
	/* Load the numbers images */
	numbers=SDL_LoadBMP("tetimg/numbers.bmp");
	if (numbers == NULL){
	 printf("Unable to load all bitmaps.\n");
	 exit(1);
    }
    
    /* Get high score data */
    hifile=fopen("hiscore.dat","r");
    if (hifile==NULL)
    {
        hifile=fopen("hiscore.dat","w");
        fputs("1 1 1",hifile);
        fclose(hifile);
        hiscore=1;
    }else{
        fscanf(hifile,"%s",tempstring);
        muson=atol(tempstring);
        fscanf(hifile,"%s",tempstring);
        sfxon=atol(tempstring);
        fscanf(hifile,"%s",tempstring);
        hiscore=atol(tempstring);
    }

    if (muson==1)
    	Mix_PlayMusic(music, -1);

    /* now loop forever */
    
    while(1){
	while (!playing) {		/*if not playing and not quitting */
					/* here we draw the main menu */
					
	/* basically the main menu works like this:  we'll fill the whole screen
	   in black, then open all the images one by one to make up the screen
	   including borders, title and buttons and checkboxes, etc.  Also we
	   draw the current high score on the right. */

	SDL_WM_SetCaption("Modest Bricks", NULL);
	SDL_FillRect(screen,NULL,SDL_MapRGB(screen->format,0,0,0));
	bitmap=SDL_LoadBMP("tetimg/title.bmp");
	if ( bitmap == NULL ) {
		printf("Unable to load all bitmaps.\n");
		exit(1);
	}
	dst.x=0;
	dst.y=0;
	dst.w=bitmap->w;
	dst.h=bitmap->h;
	SDL_BlitSurface(bitmap,NULL,screen,&dst);

	SDL_FreeSurface(bitmap);

	bitmap=SDL_LoadBMP("tetimg/play.bmp");
	if ( bitmap == NULL ) {
		printf("Unable to load all bitmaps.\n");
		exit(1);
	}
	play.x=60;
	play.y=240;
	play.w=bitmap->w;
	play.h=bitmap->h;
	SDL_BlitSurface(bitmap,NULL,screen,&play);
	SDL_FreeSurface(bitmap);

    if (muson==1)
    	bitmap=SDL_LoadBMP("tetimg/musicon.bmp");
   	else
    	bitmap=SDL_LoadBMP("tetimg/musicoff.bmp");
	if ( bitmap == NULL ) {
		printf("Unable to load all bitmaps.\n");
		exit(1);
	}
	mus.x=50;
	mus.y=185;
	mus.w=bitmap->w;
	mus.h=bitmap->h;
	SDL_BlitSurface(bitmap,NULL,screen,&mus);
	SDL_FreeSurface(bitmap);

    if (sfxon==1)
    	bitmap=SDL_LoadBMP("tetimg/sfxon.bmp");
   	else
    	bitmap=SDL_LoadBMP("tetimg/sfxoff.bmp");
	if ( bitmap == NULL ) {
		printf("Unable to load all bitmaps.\n");
		exit(1);
	}
	sfx.x=140;
	sfx.y=185;
	sfx.w=bitmap->w;
	sfx.h=bitmap->h;
	SDL_BlitSurface(bitmap,NULL,screen,&sfx);

	SDL_FreeSurface(bitmap);
	bitmap=SDL_LoadBMP("tetimg/quit.bmp");
	if ( bitmap == NULL ) {
		printf("Unable to load all bitmaps.\n");
		exit(1);
	}
	quit.x=60;
	quit.y=340;
	quit.w=bitmap->w;
	quit.h=bitmap->h;
	SDL_BlitSurface(bitmap,NULL,screen,&quit);

	SDL_FreeSurface(bitmap);
	bitmap=SDL_LoadBMP("tetimg/author.bmp");
	if ( bitmap == NULL ) {
		printf("Unable to load all bitmaps.\n");
		exit(1);
	}
	dst.x=0;
	dst.y=100;
	dst.w=bitmap->w;
	dst.h=bitmap->h;
	SDL_BlitSurface(bitmap,NULL,screen,&dst);
	
	SDL_FreeSurface(bitmap);
	bitmap=SDL_LoadBMP("tetimg/level.bmp");
	if ( bitmap == NULL ) {
		printf("Unable to load all bitmaps.\n");
		exit(1);
	}
	dst.x=270;
	dst.y=20;
	dst.w=bitmap->w;
	dst.h=bitmap->h;
	SDL_BlitSurface(bitmap,NULL,screen,&dst);
	
	SDL_FreeSurface(bitmap);
	bitmap=SDL_LoadBMP("tetimg/score.bmp");
	if ( bitmap == NULL ) {
		printf("Unable to load all bitmaps.\n");
		exit(1);
	}
	dst.x=270;
	dst.y=120;
	dst.w=bitmap->w;
	dst.h=bitmap->h;
	SDL_BlitSurface(bitmap,NULL,screen,&dst);
	SDL_FreeSurface(bitmap);
	bitmap=SDL_LoadBMP("tetimg/high.bmp");
	if ( bitmap == NULL ) {
		printf("Unable to load all bitmaps.\n");
		exit(1);
	}
	dst.x=270;
	dst.y=220;
	dst.w=bitmap->w;
	dst.h=bitmap->h;
	SDL_BlitSurface(bitmap,NULL,screen,&dst);
	SDL_FreeSurface(bitmap);
	bitmap=SDL_LoadBMP("tetimg/next.bmp");
	if ( bitmap == NULL ) {
		printf("Unable to load all bitmaps.\n");
		exit(1);
	}
	dst.x=270;
	dst.y=320;
	dst.w=bitmap->w;
	dst.h=bitmap->h;
	SDL_BlitSurface(bitmap,NULL,screen,&dst);

	SDL_FreeSurface(bitmap);
	bitmap=SDL_LoadBMP("tetimg/line.bmp");
	if ( bitmap == NULL ) {
		printf("Unable to load all bitmaps.\n");
		exit(1);
	}
	dst.x=0;
	dst.y=480;
	dst.w=bitmap->w;
	dst.h=bitmap->h;
	SDL_BlitSurface(bitmap,NULL,screen,&dst);

	SDL_FreeSurface(bitmap);
	bitmap=SDL_LoadBMP("tetimg/vline.bmp");
	if ( bitmap == NULL ) {
		printf("Unable to load all bitmaps.\n");
		exit(1);
	}
	dst.x=240;
	dst.y=0;
	dst.w=bitmap->w;
	dst.h=bitmap->h;
	SDL_BlitSurface(bitmap,NULL,screen,&dst);

   /* This is my little routine for showing the numbers in the right places.
      First we set it up to put zeros using the src.x and src.y - given the right
      width and height, this uses a rectangle on the one large image of numbers to
      pick out just a zero and put it at the dst location. */

    src.x=0;
    src.y=0;
    src.w=16;
    src.h=32;
    dst.x=350;
    dst.y=65;
    dst.w=16;
    dst.h=32;
    SDL_BlitSurface(numbers,&src,screen,&dst);
    dst.y=165;
    SDL_BlitSurface(numbers,&src,screen,&dst);

    /* To show the high score, we start from the right and put numbers going left
       until we run out of numbers in the high score. */
    dst.y=265;
    dst.x=367;
    score=hiscore;
    while(score>0)
    {
        src.x=(score%10)*16;
        dst.x=dst.x-17;
        SDL_BlitSurface(numbers,&src,screen,&dst);
        score=(long)score/10;
    }    

    /* Swap the buffers to show the screen. */        
	SDL_Flip(screen);
	menuevent=0;
	/* At main menu, waiting for an event. */
		while(!menuevent) {
			if (SDL_PollEvent(&event)>0)
			switch (event.type) {
                                /* Mouse button pressed! */
				case SDL_MOUSEBUTTONDOWN:
                     /* if they clicked in the "play" rectangle */
					if ((event.button.x>=play.x && event.button.x<=play.x+play.w) && (event.button.y>=play.y && event.button.y<=play.y+play.h)){
						playing=1;
						menuevent=1;
					}
                     /* if they clicked in the "quit" rectangle */
					if ((event.button.x>=quit.x && event.button.x<=quit.x+quit.w) && (event.button.y>=quit.y && event.button.y<=quit.y+quit.h)){
						menuevent=1;
						/* (save settings) */
                         sprintf(tempstring,"%d %d %ld",muson,sfxon,hiscore);
                         hifile=fopen("hiscore.dat","w");
                         fputs(tempstring,hifile);
			             fclose(hifile);
						goto theend;
					}
                     /* if they clicked in the "music" rectangle */
					if ((event.button.x>=mus.x && event.button.x<=mus.x+mus.w) && (event.button.y>=mus.y && event.button.y<=mus.y+mus.h)){
                        if (muson==1)
                          Mix_HaltMusic();
                        else
                          	Mix_PlayMusic(music, -1);
						muson=-muson;
						menuevent=1;
					}
					if ((event.button.x>=sfx.x && event.button.x<=sfx.x+sfx.w) && (event.button.y>=sfx.y && event.button.y<=sfx.y+sfx.h)){
                     /* if they clicked in the "sfx" rectangle */
						sfxon=-sfxon;
						menuevent=1;
					}
					break;
				case SDL_KEYUP:
					/* Keypress: Enter starts a game. */
					if (event.key.keysym.sym == SDLK_RETURN){
						playing=1;
						menuevent=1;
					}
					break;
				case SDL_QUIT:
                     /* if they quit some other way */
                    sprintf(tempstring,"%d %d %ld",muson,sfxon,hiscore);
                    hifile=fopen("hiscore.dat","w");
                    fputs(tempstring,hifile);
			        fclose(hifile);
					goto theend;
					break;
				default:
					break;
			}
		}
	   }
	   
    /* Black out the play area */
    dst.x=0;
    dst.y=0;
    dst.w=240;
    dst.h=480;
	SDL_FillRect(screen,&dst,SDL_MapRGB(screen->format,0,0,0));
	
    /* Set the game to starting conditions */
	lines=0;
	level=0;
    nextmove=0;
    score=0;

    /* Come up with the first piece */
    lookahead=(int)(7*(double)rand() / (RAND_MAX+1.0));
        switch(lookahead){
				case 0:
					nextpiece.b1x=-1;	/* Line */
					nextpiece.b1y=0;
					nextpiece.b2x=0;
					nextpiece.b2y=0;
					nextpiece.b3x=1;
					nextpiece.b3y=0;
					nextpiece.b4x=2;
					nextpiece.b4y=0;
					break;
				case 1:
					nextpiece.b1x=-1;	/* Backwards L */
					nextpiece.b1y=0;
					nextpiece.b2x=0;
					nextpiece.b2y=0;
					nextpiece.b3x=1;
					nextpiece.b3y=0;
					nextpiece.b4x=1;
					nextpiece.b4y=1;
					break;
				case 2:
					nextpiece.b1x=-1;	/* L */
					nextpiece.b1y=1;
					nextpiece.b2x=-1;
					nextpiece.b2y=0;
					nextpiece.b3x=0;
					nextpiece.b3y=0;
					nextpiece.b4x=1;
					nextpiece.b4y=0;
					break;
				case 3:
					nextpiece.b1x=0;	/* Square */
					nextpiece.b1y=0;
					nextpiece.b2x=1;
					nextpiece.b2y=0;
					nextpiece.b3x=1;
					nextpiece.b3y=-1;
					nextpiece.b4x=0;
					nextpiece.b4y=-1;
					break;
				case 4:
					nextpiece.b1x=1;	/* S */
					nextpiece.b1y=0;
					nextpiece.b2x=0;
					nextpiece.b2y=0;
					nextpiece.b3x=0;
					nextpiece.b3y=1;
					nextpiece.b4x=-1;
					nextpiece.b4y=1;
					break;
				case 5:
					nextpiece.b1x=1;	/* Z */
					nextpiece.b1y=1;
					nextpiece.b2x=0;
					nextpiece.b2y=1;
					nextpiece.b3x=0;
					nextpiece.b3y=0;
					nextpiece.b4x=-1;
					nextpiece.b4y=0;
					break;
				case 6:
					nextpiece.b1x=0;	/* T */
					nextpiece.b1y=1;
					nextpiece.b2x=-1;
					nextpiece.b2y=0;
					nextpiece.b3x=0;
					nextpiece.b3y=0;
					nextpiece.b4x=1;
					nextpiece.b4y=0;
					break;
				default:
					printf("Abnormal exit!\n");
					goto theend;
			}
/* I repeat the above code later - that's bad programming practice.  But it was easier. */

    /* Reset board. */
	for(i=0;i<10;i++)
	 for(j=0;j<21;j++)
	  board[i][j]=0;
	kD=0;

	paused=0;

	while (playing) {	// All right, they're playing.
		starting=getMS();
    /* Nextmove==0 means we need to do line checking and piece giving. */
		if (nextmove==0) {
			for(j=0;j<10;j++)
				if(board[j][1]==1)	/* Piece sticking too high - game over */
				{
                   /* Load the "you lose" sign and display, also save high score */
					SDL_FreeSurface(bitmap);
					bitmap=SDL_LoadBMP("tetimg/lose.bmp");
					if ( bitmap == NULL ) {
						printf("GAME OVER!\n");
						playing=0;
					}
					dst.x=0;
					dst.y=128;			//YOU LOSE AT GAME
					dst.w=bitmap->w;
					dst.h=bitmap->h;
					SDL_BlitSurface(bitmap,NULL,screen,&dst);
					SDL_Flip(screen);
                    sprintf(tempstring,"%d %d %ld",muson,sfxon,hiscore);
                    hifile=fopen("hiscore.dat","w");
                    fputs(tempstring,hifile);
			        fclose(hifile);
					SDL_Delay(1000);
					playing=0;
				}

			tetri=0;
         /* Check if they made any lines */
			for(i=0;i<21;i++){
				linectr=0;
				for(j=0;j<10;j++)
					if(board[j][i]!=0)
						linectr++;
				if(linectr==10)		/* A line is solid */
				{
				    tetri++;
					dst.x=0;
					dst.y=24;			/* We'll shift the area above the line to just cover it */
					src.x=0;      /* that makes it very simple! */
					src.y=0;
					src.w=240;
					src.h=24*(i-1);
					SDL_BlitSurface(screen,&src,screen,&dst);
					src.h=24;
					SDL_FillRect(screen,&src,SDL_MapRGB(screen->format,0,0,0));
                    /* and fill the top part with black too */
					for(p=i;p>0;p--)
						for(q=0;q<10;q++)
							board[q][p]=board[q][p-1];  /* now update the array */
					for(q=0;q<10;q++)
						board[q][0]=0;
					lines++;
					if (lines>9)      /* if they got 10 lines, level up! */
					{
						lines=lines-10;
						level++;
						if(level>9)
						      level=9;
						src.x=level*16;
						src.y=0;
						src.w=16;
						src.h=32;
						dst.x=350;     /* update the "level" display */
						dst.y=65;
						dst.w=16;
						dst.h=32;
						SDL_BlitSurface(numbers,&src,screen,&dst);
					}
				}
              }				
				if (tetri!=0)     /* If they made some lines */
				{				
    				if (tetri==1)    /* Scoring depends on how many you get at a time */
    				    score++;
        			else if(tetri==2)
        			    score=score+3;
        			else if(tetri==3)
        			    score=score+6;
        			else if(tetri==4)
        			    score=score+10;
 			    src.y=0;
 			    src.w=16;
 			    src.h=32;
 			    dst.x=367;
 			    dst.y=165;
 			    dst.w=16;
 			    dst.h=32;
 			    if (score>hiscore)    /* Update score and high score as needed */
 			      hiscore=score;
 			    tscore=score;
 			    while(tscore>0)
 			    {
 			        src.x=(tscore%10)*16;
 			        dst.x=dst.x-17;
 			        SDL_BlitSurface(numbers,&src,screen,&dst);
 			        tscore=(long)tscore/10;
 			        if (score==hiscore){
 			          dst.y=265;
    			        SDL_BlitSurface(numbers,&src,screen,&dst);
 			          dst.y=165;
 			      }

              }    
			}

    /* Switch to next piece and copy next piece info to current piece */
		cur=lookahead;
		piece.b1x=nextpiece.b1x;
		piece.b1y=nextpiece.b1y;
		piece.b2x=nextpiece.b2x;
		piece.b2y=nextpiece.b2y;
		piece.b3x=nextpiece.b3x;
		piece.b3y=nextpiece.b3y;
		piece.b4x=nextpiece.b4x;
		piece.b4y=nextpiece.b4y;

    /* Come up with another new next piece. */
    lookahead=(int)(7*(double)rand() / (RAND_MAX+1.0));
        switch(lookahead){
				case 0:
					nextpiece.b1x=-1;	/* Line */
					nextpiece.b1y=0;
					nextpiece.b2x=0;
					nextpiece.b2y=0;
					nextpiece.b3x=1;
					nextpiece.b3y=0;
					nextpiece.b4x=2;
					nextpiece.b4y=0;
					break;
				case 1:
					nextpiece.b1x=-1;	/* Backwards L */
					nextpiece.b1y=0;
					nextpiece.b2x=0;
					nextpiece.b2y=0;
					nextpiece.b3x=1;
					nextpiece.b3y=0;
					nextpiece.b4x=1;
					nextpiece.b4y=1;
					break;
				case 2:
					nextpiece.b1x=-1;	/* L */
					nextpiece.b1y=1;
					nextpiece.b2x=-1;
					nextpiece.b2y=0;
					nextpiece.b3x=0;
					nextpiece.b3y=0;
					nextpiece.b4x=1;
					nextpiece.b4y=0;
					break;
				case 3:
					nextpiece.b1x=0;	/* Square */
					nextpiece.b1y=0;
					nextpiece.b2x=1;
					nextpiece.b2y=0;
					nextpiece.b3x=1;
					nextpiece.b3y=-1;
					nextpiece.b4x=0;
					nextpiece.b4y=-1;
					break;
				case 4:
					nextpiece.b1x=1;	/* S */
					nextpiece.b1y=0;
					nextpiece.b2x=0;
					nextpiece.b2y=0;
					nextpiece.b3x=0;
					nextpiece.b3y=1;
					nextpiece.b4x=-1;
					nextpiece.b4y=1;
					break;
				case 5:
					nextpiece.b1x=1;	/* Z */
					nextpiece.b1y=1;
					nextpiece.b2x=0;
					nextpiece.b2y=1;
					nextpiece.b3x=0;
					nextpiece.b3y=0;
					nextpiece.b4x=-1;
					nextpiece.b4y=0;
					break;
				case 6:
					nextpiece.b1x=0;	/* T */
					nextpiece.b1y=1;
					nextpiece.b2x=-1;
					nextpiece.b2y=0;
					nextpiece.b3x=0;
					nextpiece.b3y=0;
					nextpiece.b4x=1;
					nextpiece.b4y=0;
					break;
				default:
					printf("Abnormal exit!\n");
					goto theend;
			}
			dst.x=264;
			dst.y=348;
			dst.w=116;
			dst.h=156;

    /* We're going to show the next piece right here, so fill the area in black */
    /* first, then draw it where it needs to go. */
		SDL_FillRect(screen,&dst,SDL_MapRGB(screen->format,0,0,0));
		block[0].x=298+24*nextpiece.b1x;
		block[0].y=390+24*nextpiece.b1y;
		block[0].w=24;
		block[0].h=24;
		block[1].x=298+24*nextpiece.b2x;
		block[1].y=390+24*nextpiece.b2y;
		block[1].w=24;
		block[1].h=24;
		block[2].x=298+24*nextpiece.b3x;
		block[2].y=390+24*nextpiece.b3y;
		block[2].w=24;
		block[2].h=24;
		block[3].x=298+24*nextpiece.b4x;
		block[3].y=390+24*nextpiece.b4y;
		block[3].w=24;
		block[3].h=24;
		SDL_BlitSurface(bimg[lookahead],NULL,screen,&block[0]);
		SDL_BlitSurface(bimg[lookahead],NULL,screen,&block[1]);
		SDL_BlitSurface(bimg[lookahead],NULL,screen,&block[2]);
		SDL_BlitSurface(bimg[lookahead],NULL,screen,&block[3]);

    /* Put the current piece at top of screen and draw that too */

		block[0].x=96+24*piece.b1x;
		block[0].y=24*piece.b1y;
		block[1].x=96+24*piece.b2x;
		block[1].y=24*piece.b2y;
		block[2].x=96+24*piece.b3x;
		block[2].y=24*piece.b3y;
		block[3].x=96+24*piece.b4x;
		block[3].y=24*piece.b4y;

    /* offl is offset from left, offt is offset from top */
		offl=4;
		offt=0;
		nextmove=1;

		} else {
    /* otherwise the piece has to move down - blank it out, drop it a line, draw again */
   			offt++;
			SDL_FillRect(screen,&block[0],SDL_MapRGB(screen->format,0,0,0));
			SDL_FillRect(screen,&block[1],SDL_MapRGB(screen->format,0,0,0));
			SDL_FillRect(screen,&block[2],SDL_MapRGB(screen->format,0,0,0));
			SDL_FillRect(screen,&block[3],SDL_MapRGB(screen->format,0,0,0));
			block[0].x=24*(piece.b1x+offl);
			block[0].y=24*(offt-1+piece.b1y);
			block[1].x=24*(piece.b2x+offl);
			block[1].y=24*(offt-1+piece.b2y);
			block[2].x=24*(piece.b3x+offl);
			block[2].y=24*(offt-1+piece.b3y);
			block[3].x=24*(piece.b4x+offl);
			block[3].y=24*(offt-1+piece.b4y);
    /* if we hit absolute bottom */
			if (piece.b1y+offt>19 || piece.b2y+offt>19 || piece.b3y+offt>19 || piece.b4y+offt>19 )
			{
				nextmove=0;
				board[piece.b1x+offl][piece.b1y+offt]=1;
				board[piece.b2x+offl][piece.b2y+offt]=1;
				board[piece.b3x+offl][piece.b3y+offt]=1;
				board[piece.b4x+offl][piece.b4y+offt]=1;
    /* play a sound if enabled */
				if (sfxon==1)
    				Mix_PlayChannel(-1, crash, 0);
				offt=0;
			}else if (board[piece.b1x+offl][piece.b1y+offt+1]!=0 || board[piece.b2x+offl][piece.b2y+offt+1]!=0 || board[piece.b3x+offl][piece.b3y+offt+1]!=0 || board[piece.b4x+offl][piece.b4y+offt+1]!=0){
    /* here it hit another existing piece and needs to stop */
				nextmove=0;
				board[piece.b1x+offl][piece.b1y+offt]=1;
				board[piece.b2x+offl][piece.b2y+offt]=1;
				board[piece.b3x+offl][piece.b3y+offt]=1;
				board[piece.b4x+offl][piece.b4y+offt]=1;
    /* play a sound if enabled */
				if (sfxon==1)
    				Mix_PlayChannel(-1, crash, 0);
				offt=0;
			} else {
    /* otherwise it should be able to move down */
   				block[0].y=block[0].y+24;
				block[1].y=block[1].y+24;
				block[2].y=block[2].y+24;
				block[3].y=block[3].y+24;
			}
		}
    /* draw the piece */
  		SDL_BlitSurface(bimg[cur],NULL,screen,&block[0]);
		SDL_BlitSurface(bimg[cur],NULL,screen,&block[1]);
		SDL_BlitSurface(bimg[cur],NULL,screen,&block[2]);
		SDL_BlitSurface(bimg[cur],NULL,screen,&block[3]);
		SDL_Flip(screen);

		if (nextmove != 0) {
		offt++;
    /* The piece has moved down, now we can start looking for keypresses */
		while (starting+(250-level*20)>getMS() && starting+(250-kD*220)>getMS() && nextmove!=0){
			if(paused==1) 
				starting=getMS();
			if (SDL_PollEvent(&event)>0)
			switch (event.type) {
				case SDL_KEYUP:
					if (event.key.keysym.sym == SDLK_DOWN)
    /* They pressed down so go ahead and stop looking for events, just drop us once. */
						kD=0;
					break;

				case SDL_KEYDOWN:
					if(event.key.keysym.sym==SDLK_SPACE)
					{
						if(paused==0)
						{
							SDL_WM_SetCaption("Modest Bricks - PAUSED (SPACE resumes)", NULL);
							paused=1;
						}else{
							paused=0;
							SDL_WM_SetCaption("Modest Bricks", NULL);
						}
					}
    					if (event.key.keysym.sym == SDLK_LEFT)
		if (piece.b1x+offl>0 && piece.b2x+offl>0 && piece.b3x+offl>0 && piece.b4x+offl>0)
			if(board[piece.b1x+offl-1][piece.b1y+offt]==0 && board[piece.b2x+offl-1][piece.b2y+offt]==0 && board[piece.b3x+offl-1][piece.b3y+offt]==0 && board[piece.b4x+offl-1][piece.b4y+offt]==0)
			{
				offl--;
				SDL_FillRect(screen,&block[0],SDL_MapRGB(screen->format,0,0,0));
				SDL_FillRect(screen,&block[1],SDL_MapRGB(screen->format,0,0,0));
				SDL_FillRect(screen,&block[2],SDL_MapRGB(screen->format,0,0,0));
				SDL_FillRect(screen,&block[3],SDL_MapRGB(screen->format,0,0,0));
				block[0].x=block[0].x-24;
				block[1].x=block[1].x-24;
				block[2].x=block[2].x-24;
				block[3].x=block[3].x-24;
				SDL_BlitSurface(bimg[cur],NULL,screen,&block[0]);
				SDL_BlitSurface(bimg[cur],NULL,screen,&block[1]);
				SDL_BlitSurface(bimg[cur],NULL,screen,&block[2]);
				SDL_BlitSurface(bimg[cur],NULL,screen,&block[3]);
				SDL_Flip(screen);
			}

					if (event.key.keysym.sym == SDLK_RIGHT)
        		if (piece.b1x+offl<9 && piece.b2x+offl<9 && piece.b3x+offl<9 && piece.b4x+offl<9)
			if(board[piece.b1x+offl+1][piece.b1y+offt]==0 && board[piece.b2x+offl+1][piece.b2y+offt]==0 && board[piece.b3x+offl+1][piece.b3y+offt]==0 && board[piece.b4x+offl+1][piece.b4y+offt]==0)
			{
				offl++;
				SDL_FillRect(screen,&block[0],SDL_MapRGB(screen->format,0,0,0));
				SDL_FillRect(screen,&block[1],SDL_MapRGB(screen->format,0,0,0));
				SDL_FillRect(screen,&block[2],SDL_MapRGB(screen->format,0,0,0));
				SDL_FillRect(screen,&block[3],SDL_MapRGB(screen->format,0,0,0));
				block[0].x=block[0].x+24;
				block[1].x=block[1].x+24;
				block[2].x=block[2].x+24;
				block[3].x=block[3].x+24;
				SDL_BlitSurface(bimg[cur],NULL,screen,&block[0]);
				SDL_BlitSurface(bimg[cur],NULL,screen,&block[1]);
				SDL_BlitSurface(bimg[cur],NULL,screen,&block[2]);
				SDL_BlitSurface(bimg[cur],NULL,screen,&block[3]);
				SDL_Flip(screen);
			}
					if (event.key.keysym.sym == SDLK_UP)

   		if (cur != 3 && (offt-piece.b1x<21 && offt-piece.b2x<21 && offt-piece.b3x<21 && offt-piece.b4x<21) && (piece.b1y+offl>-1 && piece.b2y+offl>-1 && piece.b3y+offl>-1 && piece.b4y+offl>-1) && (piece.b1y+offl<10 && piece.b2y+offl<10 && piece.b3y+offl<10 && piece.b4y+offl<10) && (board[piece.b1y+offl][offt-piece.b1x]==0 &&board[piece.b2y+offl][offt-piece.b2x]==0 &&board[piece.b3y+offl][offt-piece.b3x]==0 &&board[piece.b4y+offl][offt-piece.b4x]==0 ))
		{
    /* As long as they're not the square, and rotating won't hit existing pieces, */
    /* here's a cheap way to do rotates: */
			i=piece.b1x;
			piece.b1x=piece.b1y;
			piece.b1y=-i;
			i=piece.b2x;
			piece.b2x=piece.b2y;
			piece.b2y=-i;
			i=piece.b3x;
			piece.b3x=piece.b3y;
			piece.b3y=-i;
			i=piece.b4x;
			piece.b4x=piece.b4y;
			piece.b4y=-i;
			SDL_FillRect(screen,&block[0],SDL_MapRGB(screen->format,0,0,0));
			SDL_FillRect(screen,&block[1],SDL_MapRGB(screen->format,0,0,0));
			SDL_FillRect(screen,&block[2],SDL_MapRGB(screen->format,0,0,0));
			SDL_FillRect(screen,&block[3],SDL_MapRGB(screen->format,0,0,0));
			block[0].x=24*(piece.b1x+offl);
			block[0].y=24*(offt-1+piece.b1y);
			block[1].x=24*(piece.b2x+offl);
			block[1].y=24*(offt-1+piece.b2y);
			block[2].x=24*(piece.b3x+offl);
			block[2].y=24*(offt-1+piece.b3y);
			block[3].x=24*(piece.b4x+offl);
			block[3].y=24*(offt-1+piece.b4y);
			SDL_BlitSurface(bimg[cur],NULL,screen,&block[0]);
			SDL_BlitSurface(bimg[cur],NULL,screen,&block[1]);
			SDL_BlitSurface(bimg[cur],NULL,screen,&block[2]);
			SDL_BlitSurface(bimg[cur],NULL,screen,&block[3]);
			SDL_Flip(screen);
		}
					if (event.key.keysym.sym == SDLK_DOWN)
    /* They pressed down so go ahead and stop looking for events, just drop us once. */
						kD=1;
					if (event.key.keysym.sym == SDLK_ESCAPE)
					{
    /* Escape sends us back to main menu. */
                        sprintf(tempstring,"%d %d %ld",muson,sfxon,score);
				        hifile=fopen("hiscore.dat","w");
                         fputs(tempstring,hifile);
				        fclose(hifile);
						playing=0;
                    }
					break;
				case SDL_QUIT:
    /* They quit some other way.  Save our settings and high score. */
                    sprintf(tempstring,"%d %d %ld",muson,sfxon,score);
                    hifile=fopen("hiscore.dat","w");
                    fputs(tempstring,hifile);
				    fclose(hifile);
					playing=0;
					goto theend;
					break;
				default:
					break;
			}
		}
		offt--;
	}	/* while playing && !quitter */
	}}

/* That's right - I used a label and a goto.  It just made things soooooo much easier. */
/* Don't tell your teachers! */

theend:
    /* Make sure you free your surfaces, except the main screen! */
	SDL_FreeSurface(bimg[6]);
	SDL_FreeSurface(bimg[5]);
	SDL_FreeSurface(bimg[4]);
	SDL_FreeSurface(bimg[3]);
	SDL_FreeSurface(bimg[2]);
	SDL_FreeSurface(bimg[1]);
	SDL_FreeSurface(bimg[0]);
	SDL_FreeSurface(bitmap);
    /* Turn off the music and shut it all down */
  Mix_HaltMusic();
  Mix_FreeMusic(music);
  Mix_CloseAudio();
    /* Let SDL do its quitting thing */
	SDL_Quit();
}
