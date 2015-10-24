/*
 * libmm-fileinfo
 *
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Haejeong Kim <backto.kim@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_BUFFER_LEN     512
#define ADDR_BUFFER_LEN     12

#define MALLOC_STRING       "MMALLOC: addr="
#define FREE_STRING         "MEMFREE: addr="
#define PREFIX_STRING_LEN   14



int get_address(char *linebuff, char *ptrbuff)
{
	char *head = linebuff;

	if (!linebuff || !ptrbuff)
		return 0;

	head = head + PREFIX_STRING_LEN;

	while (*head != ' ') {
		*ptrbuff = *head;
		ptrbuff++;
		head++;
	}

	return 1;
}

int main(int argc, char *argv[])
{
	char linebuffer[LINE_BUFFER_LEN];
	char ptrbuffer[ADDR_BUFFER_LEN];

	int  alloccount = 0;
	int  freecount = 0;

	if (argc != 2) {
		printf("Usage: ./memtrace-read memtrace.mtr\n");
		exit(1);
	}

	FILE *fp1 = fopen(argv[1], "r");
	FILE *fp2 = fopen("memtrace-result.txt", "w");

	if (!fp1 || !fp2) {
		printf("fail to open %s\n", argv[1]);
		exit(1);
	}

	while (1) {
		memset(linebuffer, 0x00, LINE_BUFFER_LEN);
		memset(ptrbuffer, 0x00, ADDR_BUFFER_LEN);

		if (fgets(linebuffer, LINE_BUFFER_LEN, fp1) == NULL)
			break;

		if (memcmp(MALLOC_STRING, linebuffer, PREFIX_STRING_LEN) == 0) {
			get_address(linebuffer, ptrbuffer);
			alloccount++;
		}

		if (memcmp(FREE_STRING, linebuffer, PREFIX_STRING_LEN) == 0) {
			get_address(linebuffer, ptrbuffer);
			freecount++;
		}
	}

	if (alloccount != freecount) {
		char alloclist[alloccount][ADDR_BUFFER_LEN];
		int  alloccountlist[alloccount];
		char freelist[freecount][ADDR_BUFFER_LEN];
		int  freecountlist[freecount];

		int  i = 0;
		int  allocindex = 0;
		int  freeindex = 0;
		int  totalcount = 0;

		memset(alloclist, 0x00, alloccount * ADDR_BUFFER_LEN);
		memset(alloccountlist, 0x00, alloccount * 4);

		memset(freelist, 0x00, freecount * ADDR_BUFFER_LEN);
		memset(freecountlist, 0x00, freecount * 4);

		fseek(fp1, 0, SEEK_SET);

		while (1) {
			memset(linebuffer, 0x00, LINE_BUFFER_LEN);
			memset(ptrbuffer, 0x00, ADDR_BUFFER_LEN);

			if (fgets(linebuffer, LINE_BUFFER_LEN, fp1) == NULL)
				break;

			totalcount++;
			if (memcmp(MALLOC_STRING, linebuffer, PREFIX_STRING_LEN) == 0) {
				int i = 0;

				get_address(linebuffer, ptrbuffer);

				for (i = 0; i < alloccount; i++) {
					if (memcmp(ptrbuffer, alloclist[i], strlen(ptrbuffer)) == 0) {
						alloccountlist[i]++;
						break;
					}
				}

				if (i == alloccount) {
					memcpy(alloclist[allocindex], ptrbuffer, strlen(ptrbuffer));
					alloccountlist[allocindex]++;
					allocindex++;
				}
			}

			if (memcmp(FREE_STRING, linebuffer, PREFIX_STRING_LEN) == 0) {
				int i = 0;

				get_address(linebuffer, ptrbuffer);

				for (i = 0; i < freecount; i++) {
					if (memcmp(ptrbuffer, freelist[i], strlen(ptrbuffer)) == 0) {
						freecountlist[i]++;
						break;
					}
				}

				if (i == freecount) {
					memcpy(freelist[freeindex], ptrbuffer, strlen(ptrbuffer));
					freecountlist[freeindex]++;
					freeindex++;
				}
			}
		}

		printf("Total: %d mem operation\n", totalcount);

		int i1 = 0, i2 = 0;


		fprintf(fp2, "-------------------------------------------------------------\n");
		fprintf(fp2, "ADDRESS (malloc count, free cout, diff)\n");


		for (i1 = 0; i1 < allocindex; i1++) {
			for (i2 = 0; i2 < freeindex; i2++) {
				if (strcmp(alloclist[i1], freelist[i2]) == 0) {
					if (strcmp(alloclist[i1], "Checked") != 0)
						break;
				}
			}

			if (i2 == freeindex) {
				/*    fprintf (fp2, "%s error\n", alloclist[i1]); */
			} else {
				fprintf(fp2, "%s %12d %8d %8d\n", alloclist[i1], alloccountlist[i1], freecountlist[i2], alloccountlist[i1] - freecountlist[i2]);
				strcpy(alloclist[i1], "Checked");
				strcpy(freelist[i2], "Checked");
			}
		}

		for (i = 0; i < allocindex; i++) {
			if (strcmp(alloclist[i], "Checked") != 0)
				fprintf(fp2, "%s error\n", alloclist[i]);
		}

		for (i = 0; i < freeindex; i++) {
			if (strcmp(freelist[i], "Checked") != 0)
				fprintf(fp2, "%s error\n", freelist[i]);
		}
	}

	fclose(fp1);
	fclose(fp2);

	exit(0);
}

