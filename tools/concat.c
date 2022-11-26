#include <stdio.h>
#include <stdlib.h>

/* Damn dummy file concatter */
int main(int argc, char **argv)
{
	FILE *result = fopen(argv[1], "w");
	if (result == NULL)
	{
		printf("Could not open result file");
		return 1;
	}

	for (int i = 2, ch = 0; i < argc; ++i)
	{
		FILE *f = fopen(argv[i], "r");
		if (f == NULL)
		{
			printf("Could not open file: %s", argv[i]);
			return 1;
		}
		while ((ch = fgetc(f)) != EOF)
		{
			fputc(ch, result);
		}
		fclose(f);
	}

	fclose(result);

	return 0; 
}
