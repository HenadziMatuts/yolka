#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>

/*
              /\
             /.o\
            /_.._\
             /.o\
            /o...\
           /...o..\
          /__o...__\
            /..o.\
           /.o...o\
          /o...o...\
         /___o...___\
             |__|

*/

#define RED             "\e[31m"
#define GREEN           "\e[32m"
#define YELLOW          "\e[33m"
#define WHITE           "\e[37m"
#define BLACK           "\e[40m"
#define GREY            "\e[90m"
#define BRIGHT_RED      "\e[91m"
#define BRIGHT_GREEN    "\e[92m"
#define BRIGHT_YELLOW   "\e[93m"
#define RESET           "\e[0m"

typedef enum drawables_t
{
	TEXT = 0,

	BACKGROUND = 160,
	TREE_OUTLINE_SLASH,
	TREE_OUTLINE_BACKSLASH,
	TREE_OUTLINE_UNDERSCORE,
	TREE_OUTLINE_TILDE,
	TREE_INLINE,
	TREE_TRUNK_PIPE,
	TREE_TRUNK_UDERSCORE,
	TREE_TOP_LEFT,
	TREE_TOP_RIGHT,

	TREE_PEEK_FRAME_1_LEFT,
	TREE_PEEK_FRAME_1_RIGHT,
	TREE_PEEK_FRAME_2,
	TREE_PEEK_FRAME_3,
	TREE_PEEK_FRAME_4,

	LED_RED,
	LED_GREEN,
	LED_YELLOW,

	SNOWFLAKE_BRIGHT,
	SNOWFLAKE_DARK

} drawables_t;

typedef enum layers_t
{
	LAYERS_TREE = 0,
	LAYERS_SNOW,
	LAYERS_LED,
	LAYERS_TEXT,
	LAYERS_TOTAL

} layers_t;

drawables_t **layers[LAYERS_TOTAL];

int cascades = 3;
int first_cascade_height = 3;
char *greeting = "";

int *cascade_heights = NULL;

int width = 0;
int height = 11;

int snowdrift_height_cap = 3;

int frame = 0;

const char* drawables_to_string(drawables_t drawable)
{
	switch (drawable)
	{
		case BACKGROUND:              return " ";
		case TREE_OUTLINE_SLASH:      return GREEN"/";
		case TREE_OUTLINE_BACKSLASH:  return GREEN"\\";
		case TREE_OUTLINE_UNDERSCORE: return GREEN"_";
		case TREE_OUTLINE_TILDE:      return GREEN"~";
		case TREE_INLINE:             return GREEN"\u00b7";
		case TREE_TRUNK_PIPE:         return RED"|";
		case TREE_TRUNK_UDERSCORE:    return RED"_";

		case TREE_PEEK_FRAME_1_LEFT:  return YELLOW"}";
		case TREE_PEEK_FRAME_1_RIGHT: return YELLOW"{";
		case TREE_PEEK_FRAME_2:       return YELLOW"\\";
		case TREE_PEEK_FRAME_3:       return YELLOW"=";
		case TREE_PEEK_FRAME_4:       return YELLOW"/";
		
		case LED_RED:                 return BRIGHT_RED"\u2218";
		case LED_GREEN:               return BRIGHT_GREEN"\u2218";
		case LED_YELLOW:              return BRIGHT_YELLOW"\u2218";
		case SNOWFLAKE_BRIGHT:        return WHITE"*";
		case SNOWFLAKE_DARK:          return GREY"*";
	}
}

void reset_coursor()
{
	fprintf(stdout, "\r");
	for(int i = 0; i < height; i++)
	{
		fprintf(stdout, "\033[A");
	}
}

void render()
{
	drawables_t **tree = layers[LAYERS_TREE];
	drawables_t **snow = layers[LAYERS_SNOW];
	drawables_t **led = layers[LAYERS_LED];
	drawables_t **text = layers[LAYERS_TEXT];

	for (int i = 0; i < height; i++)
	{
		/* line background */
		fprintf (stdout, BLACK);

		for (int j = 0; j < width; j++)
		{
			if (text[i][j] != BACKGROUND)
			{
				fprintf(stdout, "%s%c", BRIGHT_YELLOW, greeting[text[i][j]]);
			}
			else if (snow[i][j] != BACKGROUND)
			{
				/* snow layer on top */
				fprintf(stdout, "%s", drawables_to_string(snow[i][j]));
			}
			else if (tree[i][j] == TREE_INLINE && led[i][j] != BACKGROUND)
			{
				/* led is in the middle */
				fprintf(stdout, "%s", drawables_to_string(led[i][j]));
			}
			else
			{
				/* tree itself is on the last layer */
				fprintf(stdout, "%s", drawables_to_string(tree[i][j]));
			}
		}

		fprintf(stdout, RESET"\n");
	}

	reset_coursor();
}

void update_led_layer()
{
	drawables_t **led = layers[LAYERS_LED];
	drawables_t base = led[0][3];
	drawables_t top, middle, bottom;

	if (frame % 4)
	{
		return;
	}

	switch (base)
	{
		case LED_RED:
			top = LED_YELLOW;
			middle = LED_RED;
			bottom = LED_GREEN;
			break;

		case LED_GREEN:
			top = LED_RED;
			middle = LED_GREEN;
			bottom = LED_YELLOW;
			break;

		case LED_YELLOW:
			top = LED_GREEN;
			middle = LED_YELLOW;
			bottom = LED_RED;
			break;
	}

	for (int i = 0; i < height; i += 3)
	{
		for (int j = (i % 2) ? 0 : 3; j < width; j += 6)
		{
			led[i][j] = top;
			if (i + 2 < height)
			{
				led[i + 2][j] = bottom;
			}
		}
	}

	for (int i = 1; i < height; i += 3)
	{
		for (int j = (i % 2) ? 0 : 3; j < width; j += 6)
		{
			led[i][j] = middle;
		}
	}
}

bool is_the_ground_fully_covered_with_snow()
{
	drawables_t **snow = layers[LAYERS_SNOW];

	for (int i = 0; i < width; i++)
	{
		if (snow[height - 1][i] == BACKGROUND)
		{
			return false;
		}
	}

	return true;
}

bool is_there_are_any_snowlakes_within_the_first_line_of_the_snow_layer()
{
	drawables_t **snow = layers[LAYERS_SNOW];

	for (int i = 0; i < width; i++)
	{
		if (snow[0][i] != BACKGROUND)
		{
			return true;
		}
	}

	return false;
}

void new_snowflake()
{
	drawables_t **snow = layers[LAYERS_SNOW];

	if (!is_there_are_any_snowlakes_within_the_first_line_of_the_snow_layer())
	{
		snow[0][rand() % width] = (rand() % 2) ?
			((rand() % 2) ? SNOWFLAKE_BRIGHT : SNOWFLAKE_DARK) :
			BACKGROUND;
	}
}

void update_snow_layer()
{
	drawables_t **snow = layers[LAYERS_SNOW];

	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			if (snow[j][i] != BACKGROUND)
			{
				if (j == height - 1)
				{	
					/* reached the ground */
					snow[j][i] = SNOWFLAKE_BRIGHT;
					new_snowflake();
				}
				else if ((snow[j + 1][i] == BACKGROUND) ||
					((snow[j + 1][i] != BACKGROUND) &&
						(j < height - snowdrift_height_cap)))
				{
					/* falling */
					snow[j][i] = BACKGROUND;
					snow[(++j) % height][i] = (rand() % 3) ?
						SNOWFLAKE_BRIGHT : SNOWFLAKE_DARK;
				}
				else
				{
					/* idle within the snowdrift */
					snow[j][i] = SNOWFLAKE_BRIGHT;
				}
			}
		}
	}

	if (is_the_ground_fully_covered_with_snow())
	{
		for (int i = 0; i < width; i++)
		{
			snow[height - 1][i] = BACKGROUND;
		}
	}
}

void update_tree_layer()
{
	if (frame % 3)
	{
		return;
	}

	int peek = 7, center = (width / 2);

	drawables_t **tree = layers[LAYERS_TREE];
	drawables_t base = tree[peek][center];

	switch(base)
	{
		case TREE_PEEK_FRAME_1_RIGHT:
			tree[peek][center] =
				tree[peek][center - 1] = TREE_PEEK_FRAME_2;
			break;

		case TREE_PEEK_FRAME_2:
			tree[peek][center] =
				tree[peek][center - 1] = TREE_PEEK_FRAME_3;
			break;

		case TREE_PEEK_FRAME_3:
			tree[peek][center] =
				tree[peek][center - 1] = TREE_PEEK_FRAME_4;
			break;

		case TREE_PEEK_FRAME_4:
			tree[peek][center] = TREE_PEEK_FRAME_1_RIGHT;
			tree[peek][center - 1] = TREE_PEEK_FRAME_1_LEFT;
			break;
	}
}

void update()
{
	frame++;

	update_tree_layer();
	update_snow_layer();
	update_led_layer();
}

void initialize_text_layer()
{
	drawables_t **text = layers[LAYERS_TEXT];
	size_t len;

	if (!(len = strlen(greeting)))
	{
		return;
	}

	int left_edge = (width - len) / 2;

	for(size_t i = 0; i < len; i++)
	{
		text[3][left_edge++] = i;
	}
}

void initialize_led_layer()
{
	drawables_t **led = layers[LAYERS_LED];

	for (int i = 0; i < height; i += 3)
	{
		for (int j = (i % 2) ? 0 : 3; j < width; j += 6)
		{
			led[i][j] = LED_RED;
			if(i + 2 < height)
			{
				led[i + 2][j] = LED_YELLOW;	
			}
		}
	}

	for (int i = 1; i < height; i += 3)
	{
		for(int j = (i % 2) ? 0 : 3; j < width; j += 6)
		{
			led[i][j] = LED_GREEN;
		}
	}
}

void initialize_snow_layer()
{
	drawables_t **snow = layers[LAYERS_SNOW];

	for (int i = 0; i < width; i++)
	{
		snow[rand() % height][i] =
			(rand() % 2) ? SNOWFLAKE_BRIGHT : SNOWFLAKE_DARK; 
	}
}

void initialize_tree_layer()
{
	int curr_height = 8;
	int interval = 0;
	int center = (width / 2);
	int left = 0;
	int right = 0;

	drawables_t **tree = layers[LAYERS_TREE];

	tree[curr_height - 1][center - 1] = TREE_PEEK_FRAME_1_LEFT;
	tree[curr_height - 1][center] = TREE_PEEK_FRAME_1_RIGHT;

	for(int i = 0; i < cascades;)
	{
		int next_height = curr_height + cascade_heights[i];

		for(curr_height; curr_height < next_height; curr_height++)
		{
			left = center - interval - 1;
			right = center + interval;

			tree[curr_height][left] = TREE_OUTLINE_SLASH;
			tree[curr_height][right] = TREE_OUTLINE_BACKSLASH;

			for(int j = left + 1; j < right; j++)
			{
				tree[curr_height][j] = TREE_INLINE;
			}

			interval++;
		}

		i++;

		if(i == cascades)
		{
			tree[curr_height - 1][left] =
				tree[curr_height - 1][right] = BACKGROUND;

			for(int j = left + 1; j < right; j++)
			{
				tree[curr_height - 1][j] = TREE_OUTLINE_TILDE;
			}
		}
		else
		{
			for(int j = 1; j <= i; j++)
			{
				tree[curr_height - 1][left + j] = TREE_OUTLINE_UNDERSCORE;
				tree[curr_height - 1][right - j] = TREE_OUTLINE_UNDERSCORE;
			}
		}

		interval = interval - (i*2) + (i-1);
	}

	if((interval /= 2) % 2)
	{
		interval += 1;
	}

	left = center - interval - 1;
	right = center + interval;

	tree[curr_height][left] =
		tree[curr_height][right] = TREE_TRUNK_PIPE;

	for(int j = left + 1; j < right; j++)
	{
		tree[curr_height][j] = TREE_TRUNK_UDERSCORE;
			
	}
}

void help()
{
	fprintf(stdout,
		"Yolka. Author: Henadzi Matuts, 2017-2018.\n"
		"usage:\n"
		"\tyolka [--help|-h] [--cascades|-c num] [--height|-h num] [--greeting|-g text]\n"
		"\t\"cascades\" stands for amount of tree cascades, default: 3\n"
		"\t\"height\" stands for first cascade height, default: 3, min: 3\n"
		"\t\"greeting\" length is limited to \"screen\" width minus two (160 chracters max)\n\n"
		"***Happy 2018 Year!***\n");
}

void initialize()
{
	srand(time(NULL));

	/* tree structure */
	cascade_heights = calloc(cascades, sizeof(int));
	cascade_heights[0] = first_cascade_height;
	
	for(int i = 1; i < cascades; i++)
	{
		cascade_heights[i] =
			cascade_heights[i - 1] + 3;
	}

	/* "screen" width and height */
	for (int i = 0; i < cascades; i++)
	{
		height += cascade_heights[i];
	}
	if ((width = height * 2) % 2)
	{
		width -= 1;
	}

	if (strlen(greeting) > width - 2)
	{
		help();
		exit(1);
	}

	for (int i = 0; i < LAYERS_TOTAL; i++)
	{
		layers[i] = (drawables_t**)malloc(height * sizeof(drawables_t*));
		for (int j = 0; j < height; j++)
		{
			layers[i][j] = (drawables_t*)calloc(width, sizeof(drawables_t));
			for (int k = 0; k < width; k++)
			{
				layers[i][j][k] = BACKGROUND;
			}
		}
	}

	initialize_tree_layer();
	initialize_snow_layer();
	initialize_led_layer();
	initialize_text_layer();
}

#define MAIN_LOOP while(1)
#define DELAY_USEC 75000

static struct option options[] = {
	{"help", no_argument, NULL, 'h'},
	{"cascades", required_argument, NULL, 'c'},
	{"height", required_argument, NULL, 'H'},
	{"greeting", required_argument, NULL, 'g'},
	{0, 0, 0, 0},
};

static const char *optstring = "hc:H:g:";

void parse_options(int argc, char *argv[])
{
	while (1)
	{
		switch (getopt_long(argc, argv, optstring, options, NULL))
		{
		case EOF:
			break;

		case 'h':
			help();
			exit(0);

		case 'c':
			if(!(cascades = atoi(optarg)))
			{
				help();
				exit(1);
			}
			continue;

		case 'H':
			if(!(first_cascade_height = atoi(optarg))
				|| first_cascade_height < 3)
			{
				help();
				exit(1);
			}
			continue;

		case 'g':
			if (strlen(optarg) > 160)
			{
				help();
				exit(1);
			}
			greeting = optarg;
			continue;

		default:
			help();
			exit(1);
		}

		break;
	}
}

int main(int argc, char *argv[])
{
	parse_options(argc, argv);

	initialize();

	MAIN_LOOP
	{
		update();
		render();

		usleep(DELAY_USEC);
	}

	return 0;
}
