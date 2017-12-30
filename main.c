#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
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

typedef enum drawables_t
{
	BACKGROUND = 0,
	TREE_OUTLINE_SLASH,
	TREE_OUTLINE_BACKSLASH,
	TREE_OUTLINE_UNDERSCORE,
	TREE_INLINE,
	TREE_ROOT_PIPE,
	TREE_ROOT_UDERSCORE,
	TREE_TOP_LEFT,
	TREE_TOP_RIGHT,

	LIGHTINGS_RED,
	LIGHTINGS_GREEN,
	LIGHTINGS_YELLOW,

	SNOWFLAKE,

	SNOWFLAKE_BRIGHT,
	SNOWFLAKE_DARK

} drawables_t;

int cascades = 3;
int *cascade_height = NULL;

int width = 0;
int height = 11;

drawables_t **tree_layer;
drawables_t **snow_layer;
drawables_t **lightings_layer;


const char* drawable_to_string(drawables_t drawable)
{
	switch(drawable)
	{
		case BACKGROUND:
			return " ";

		case TREE_OUTLINE_SLASH:
			return "\e[32m/";

		case TREE_OUTLINE_BACKSLASH:
			return "\e[32m\\";

		case TREE_OUTLINE_UNDERSCORE:
			return "\e[32m_";

		case TREE_INLINE:
			return "\e[32m.";

		case TREE_ROOT_PIPE:
			return "\e[31m|";

		case TREE_ROOT_UDERSCORE:
			return "\e[31m_";

		case TREE_TOP_LEFT:
			return "\e[33m}";

		case TREE_TOP_RIGHT:
			return "\e[33m{";

		case LIGHTINGS_RED:
			return "\e[91m\u00ba";

		case LIGHTINGS_GREEN:
			return "\e[92m\u00ba";

		case LIGHTINGS_YELLOW:
			return "\e[93m\u00ba";

		case SNOWFLAKE_BRIGHT:
			return "\e[37m*";

		case SNOWFLAKE_DARK:
			return "\e[90m*";
	}
}


void render()
{
	for(int i = 0; i < height; i++)
	{
		fprintf(stdout, "\e[40m");

		for(int j = 0; j < width; j++)
		{
			drawables_t snowflake = snow_layer[i][j];
			if(snowflake > SNOWFLAKE)
			{
				fprintf(stdout, "%s", drawable_to_string(snowflake));
			}
			else
			{
				if(tree_layer[i][j] == TREE_INLINE)
				{
					if(lightings_layer[i][j] != BACKGROUND)
					{
						fprintf(stdout, "%s", drawable_to_string(lightings_layer[i][j]));
					}
					else
					{
						fprintf(stdout, "%s", drawable_to_string(tree_layer[i][j]));
					}
				}
				else
				{
					fprintf(stdout, "%s", drawable_to_string(tree_layer[i][j]));
				}
			}
		}

		fprintf(stdout, "\e[0m\n");
	}

	fprintf(stdout, "\r");
	for(int i = 0; i < height; i++)
	{
		fprintf(stdout, "\033[A");
	}
}

void new_snowflake()
{
	int generate = 1;

	for(int i = 0; i < width; i++)
	{
		if(snow_layer[0][i] > SNOWFLAKE)
		{
			generate = 0;
			break;
		}
	}

	if(generate)
	{
		snow_layer[0][rand() % width] = (rand() % 2) ?
			((rand() % 2) ? SNOWFLAKE_BRIGHT : SNOWFLAKE_DARK) :
			BACKGROUND;
	}
}

void update_lightings_layer()
{

	static int frames = 0;

	if(frames < 3)
	{
		frames++;
		return;
	}
	else
	{
		frames = 0;
	}

	drawables_t light = lightings_layer[0][3];
	drawables_t top, middle, bottom;

	switch(light)
	{
		case LIGHTINGS_RED:
			top = LIGHTINGS_YELLOW;
			middle = LIGHTINGS_RED;
			bottom = LIGHTINGS_GREEN;
			break;

		case LIGHTINGS_GREEN:
			top = LIGHTINGS_RED;
			middle = LIGHTINGS_GREEN;
			bottom = LIGHTINGS_YELLOW;
			break;

		case LIGHTINGS_YELLOW:
			top = LIGHTINGS_GREEN;
			middle = LIGHTINGS_YELLOW;
			bottom = LIGHTINGS_RED;
			break;
	}

	for(int i = 0; i < height; i += 3)
	{
		for(int j = (i % 2) ? 0 : 3; j < width; j += 6)
		{
			lightings_layer[i][j] = top;
			lightings_layer[i + 2][j] = bottom;
		}
	}

	for(int i = 1; i < height; i += 3)
	{
		for(int j = (i % 2) ? 0 : 3; j < width; j += 6)
		{
			lightings_layer[i][j] = middle;
		}
	}
}

void update_snow_layer()
{
	/* updating snowfall */
	for(int i = 0; i < width; i++)
	{
		for(int j = 0; j < height; j++)
		{
			if(snow_layer[j][i] > SNOWFLAKE)
			{
				if(j == height - 1)
				{	
					snow_layer[j][i] = SNOWFLAKE_BRIGHT;
					new_snowflake();
				}
				else if((snow_layer[j + 1][i] < SNOWFLAKE) ||
					((snow_layer[j + 1][i] > SNOWFLAKE) && (j < height - 3)))
				{
					snow_layer[j][i] = BACKGROUND;
					snow_layer[(j + 1) % height][i] = (rand() % 2) ?
						SNOWFLAKE_BRIGHT : SNOWFLAKE_DARK;
					j++;
				}
				else
				{
					snow_layer[j][i] = SNOWFLAKE_BRIGHT;
				}
			}
		}
	}

	int filled = 1;
	for(int i = 0; i < width; i++)
	{
		if(snow_layer[height - 1][i] == BACKGROUND)
		{
			filled = 0;
			break;
		}
	}

	if(filled)
	{
		for(int i = 0; i < width; i++)
		{
			snow_layer[height - 1][i] = BACKGROUND;
		}
	}
}

void update()
{
	update_snow_layer();
	update_lightings_layer();
}

void generate_lightings()
{
	for(int i = 0; i < height; i += 3)
	{
		for(int j = (i % 2) ? 0 : 3; j < width; j += 6)
		{
			lightings_layer[i][j] = LIGHTINGS_RED;
			lightings_layer[i + 2][j] = LIGHTINGS_YELLOW;
		}
	}

	for(int i = 1; i < height; i += 3)
	{
		for(int j = (i % 2) ? 0 : 3; j < width; j += 6)
		{
			lightings_layer[i][j] = LIGHTINGS_GREEN;
		}
	}
}

void genetate_snow()
{
	for(int i = 0; i < width; i++)
	{
		snow_layer[rand() % height][i] =
			(rand() % 2) ? SNOWFLAKE_BRIGHT : SNOWFLAKE_DARK; 
	}
}

void generate_tree()
{
	int curr_height = 8;
	int interval = 0;
	int center = (width / 2);
	int left = 0;
	int right = 0;

	tree_layer[curr_height - 1][center - 1] = TREE_TOP_LEFT;
	tree_layer[curr_height - 1][center] = TREE_TOP_RIGHT;

	for(int i = 0; i < cascades;)
	{
		int next_height = curr_height + cascade_height[i];

		for(curr_height; curr_height < next_height; curr_height++)
		{
			left = center - interval - 1;
			right = center + interval;

			tree_layer[curr_height][left] = TREE_OUTLINE_SLASH;
			tree_layer[curr_height][right] = TREE_OUTLINE_BACKSLASH;

			for(int j = left + 1; j < right; j++)
			{
				tree_layer[curr_height][j] = TREE_INLINE;
			}

			interval++;
		}

		i++;

		if(i == cascades)
		{
			for(int j = left + 1; j < right; j++)
			{
				tree_layer[curr_height - 1][j] = TREE_OUTLINE_UNDERSCORE;
			}
		}
		else
		{
			for(int j = 1; j <= i; j++)
			{
				tree_layer[curr_height - 1][left + j] = TREE_OUTLINE_UNDERSCORE;
				tree_layer[curr_height - 1][right - j] = TREE_OUTLINE_UNDERSCORE;
			}
		}

		interval -= i * 2;
	}

	left = center - interval - 1;
	right = center + interval;

	tree_layer[curr_height][left] =
		tree_layer[curr_height][right] = TREE_ROOT_PIPE;

	for(int j = left + 1; j < right; j++)
	{
		tree_layer[curr_height][j] = TREE_ROOT_UDERSCORE;
		tree_layer[curr_height - 1][j] = TREE_INLINE;
			
	}
}

void initialize()
{
	srand(time(NULL));

	/* tree structure */
	cascade_height = calloc(cascades, sizeof(int));

	cascade_height[0] = 7;
	cascade_height[1] = 8;
	cascade_height[2] = 10;

	/* scren width and height */
	for(int i = 0; i < cascades; i++)
	{
		height += cascade_height[i];
	}

	width = height * 2;

	if(width % 2)
	{
		width -= 1;
	}

	/* tree layout */
	tree_layer = (drawables_t**)malloc(height * sizeof(drawables_t*));
	for(int i = 0; i < height; i++)
	{
		tree_layer[i] = (drawables_t*)calloc(width, sizeof(drawables_t));
	}
 
	/* snowfall layout */
	snow_layer = (drawables_t**)malloc(height * sizeof(drawables_t*));
	for(int i = 0; i < height; i++)
	{
		snow_layer[i] = (drawables_t*)calloc(width, sizeof(drawables_t));
	}

	/* lightings layout */
	lightings_layer = (drawables_t**)malloc(height * sizeof(drawables_t*));
	for(int i = 0; i < height; i++)
	{
		lightings_layer[i] = (drawables_t*)calloc(width, sizeof(drawables_t));
	}

	generate_tree();
	genetate_snow();
	generate_lightings();
}

int main(int argc, char *argv[])
{
	initialize();

	while(1)
	{
		update();
		render();
		usleep(75000);
	}

	return 0;
}
