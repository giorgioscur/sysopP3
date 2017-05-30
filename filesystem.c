#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include "libdisksimul.h"
#include "filesystem.h"

/**
 * @brief Format disk.
 * 
 */
int fs_format(){
	int ret, i;
	struct root_table_directory root_dir;
	struct sector_data sector;
	
	if ( (ret = ds_init(FILENAME, SECTOR_SIZE, NUMBER_OF_SECTORS, 1)) != 0 ){
		return ret;
	}
	
	memset(&root_dir, 0, sizeof(root_dir));
	
	root_dir.free_sectors_list = 1; /* first free sector. */
	
	ds_write_sector(0, (void*)&root_dir, SECTOR_SIZE);
	
	/* Create a list of free sectors. */
	memset(&sector, 0, sizeof(sector));
	
	for(i=1;i<NUMBER_OF_SECTORS;i++){
		if(i<NUMBER_OF_SECTORS-1){
			sector.next_sector = i+1;
		}else{
			sector.next_sector = 0;
		}
		ds_write_sector(i, (void*)&sector, SECTOR_SIZE);
	}
	
	ds_stop();
	
	printf("Disk size %d kbytes, %d sectors.\n", (SECTOR_SIZE*NUMBER_OF_SECTORS)/1024, NUMBER_OF_SECTORS);
	
	return 0;
}

/**
 * @brief Create a new file on the simulated filesystem.
 * @param input_file Source file path.
 * @param simul_file Destination file path on the simulated file system.
 * @return 0 on success.
 */
int fs_create(char* input_file, char* simul_file){
	int ret, tam;
    struct root_table_directory root_dir;
	struct sector_data sector;
    char data[100];
    FILE *ptr;

	if ( (ret = ds_init(FILENAME, SECTOR_SIZE, NUMBER_OF_SECTORS, 0)) != 0 ){
		return ret;
	}
    
    ds_read_sector(0,(void*)&root_dir, SECTOR_SIZE);
    ds_read_sector(root_dir.free_sectors_list,(void*)&sector, SECTOR_SIZE);

    ptr = fopen (input_file,"r");
    fread(&data, sizeof(char), 100,ptr);    
    printf("\nO conteúdo do arquivo é':\n %s \n", data);
    
    fseek (ptr, 0, SEEK_END);
    tam = ftell (ptr);

    printf("\nTamanho:\n %d \n", tam);

    fclose(ptr);

    root_dir.entries[root_dir.free_sectors_list].dir = 0;
    strcpy(root_dir.entries[root_dir.free_sectors_list].name, simul_file);
    root_dir.entries[root_dir.free_sectors_list].size_bytes = 2;  
    root_dir.entries[root_dir.free_sectors_list].sector_start = root_dir.free_sectors_list;
    
    strcpy(sector.data,data);

    ds_write_sector(root_dir.free_sectors_list,(void*)&sector, SECTOR_SIZE);

	ds_stop();
	
	return 0;
}

/**
 * @brief Read file from the simulated filesystem.
 * @param output_file Output file path.
 * @param simul_file Source file path from the simulated file system.
 * @return 0 on success.
 */
int fs_read(char* output_file, char* simul_file){
	int ret;
	if ( (ret = ds_init(FILENAME, SECTOR_SIZE, NUMBER_OF_SECTORS, 0)) != 0 ){
		return ret;
	}
	
	/* Write the code to read a file from the simulated filesystem. */
	
	ds_stop();
	
	return 0;
}

/**
 * @brief Delete file from file system.
 * @param simul_file Source file path.
 * @return 0 on success.
 */
int fs_del(char* simul_file){
	int ret;
	if ( (ret = ds_init(FILENAME, SECTOR_SIZE, NUMBER_OF_SECTORS, 0)) != 0 ){
		return ret;
	}
	
	/* Write the code delete a file from the simulated filesystem. */
	
	ds_stop();
	
	return 0;
}

/**
 * @brief List files from a directory.
 * @param simul_file Source file path.
 * @return 0 on success.
 */
int fs_ls(char *dir_path){
	int ret;
	if ( (ret = ds_init(FILENAME, SECTOR_SIZE, NUMBER_OF_SECTORS, 0)) != 0 ){
		return ret;
	}
	
	/* Write the code to show files or directories. */
	
	ds_stop();
	
	return 0;
}

/**
 * @brief Create a new directory on the simulated filesystem.
 * @param directory_path directory path.
 * @return 0 on success.
 */
int fs_mkdir(char* directory_path){
	// int ret, i, success;
	// struct root_table_directory root_dir;
	// struct sector_data sector;
	// struct table_directory directory;
	// char* nome;

	// /* Initialize Virtual Memory */
	// if ( (ret = ds_init(FILENAME, SECTOR_SIZE, NUMBER_OF_SECTORS, 0)) != 0 ){
	// 	return ret;
	// }
	
	// /* Code to create a new directory. */
	// ds_read_sector(0,(void*)&root_dir, SECTOR_SIZE); //Read root dir
	
	// /* Code to travel to directory*/
	// if( (ret = change_directory(directory_path,(void*)&directory, &nome)) != 0) {
	// 	for(i= 0;i<16;i++) {
	// 		if(directory.entries[i].sector_start == 0) {
	// 			directory.entries[i].dir = 1;
	// 			directory.entries[i].name = *nome;
	// 			directory.entries[i].size_bytes = 0;
	// 			directory.entries[i].sector_start = root_dir.free_sectors_list;
	// 			success = 1;
	// 		}
	// 	}
	// } else {
	// 	for(i= 0;i<15;i++) {
	// 		if(root_dir.entries[i].sector_start == 0) {
	// 			root_dir.entries[i].dir = 1;
	// 			root_dir.entries[i].name = *nome;
	// 			root_dir.entries[i].size_bytes = 0;
	// 			root_dir.entries[i].sector_start = root_dir.free_sectors_list;
	// 			success = 1;
	// 		}
	// 	}
	// }
	// if(success){
	// 	root_dir.free_sectors_list++
	// } else {
	// 	printf("Error");
	// }

	// ds_write_sector(0, (void*)&root_dir, SECTOR_SIZE); // save root dir
	// ds_stop();
	
	// return 0;
}

int change_directory(char* directory_path, void* directory_r, char *nome) { //Tem que testar isso, ficou uma confusao esses ponteiros
	// char* pch = strtok(directory_path, "/");
	// char* name;
	// struct root_table_directory root_dir;
	// struct table_directory directory;

	// ds_read_sector(0,(void*)&root_dir, SECTOR_SIZE); //Read root dir
	// for(i=0; i<15; i++) {
	// 	if (strcmp(root_dir.entries[i].name, pch) == 0) { //Isso ta meio feio, deve ter como fazer mais bonito
	// 		ds_read_sector(root_dir.entries[i].sector_start,(void*)&directory,SECTOR_SIZE);

	// 		do{
	// 			for(i=0; i<16; i++) {
	// 				if(strcmp(directory.entries[i].name,pch) == 0) {
	// 					ds_read_sector(directory.entries[i].sector_start,(void*)&directory,SECTOR_SIZE);
	// 					name = pch;
	// 				}
	// 			}
	// 		} while( (pch = strtok(NULL, "/")) != NULL);

	// 		*directory_r = directory;
	// 		*nome = name;
	// 		return 1;
	// 	}
	// 	return 0; 
	// } 
	
}

/**
 * @brief Remove directory from the simulated filesystem.
 * @param directory_path directory path.
 * @return 0 on success.
 */
int fs_rmdir(char *directory_path){
	int ret;
	if ( (ret = ds_init(FILENAME, SECTOR_SIZE, NUMBER_OF_SECTORS, 0)) != 0 ){
		return ret;
	}
	
	/* Write the code to delete a directory. */
	
	ds_stop();
	
	return 0;
}

/**
 * @brief Generate a map of used/available sectors. 
 * @param log_f Log file with the sector map.
 * @return 0 on success.
 */
int fs_free_map(char *log_f){
	int ret, i, next;
	struct root_table_directory root_dir;
	struct sector_data sector;
	char *sector_array;
	FILE* log;
	int pid, status;
	int free_space = 0;
	char* exec_params[] = {"gnuplot", "sector_map.gnuplot" , NULL};

	if ( (ret = ds_init(FILENAME, SECTOR_SIZE, NUMBER_OF_SECTORS, 0)) != 0 ){
		return ret;
	}
	
	/* each byte represents a sector. */
	sector_array = (char*)malloc(NUMBER_OF_SECTORS);

	/* set 0 to all sectors. Zero means that the sector is used. */
	memset(sector_array, 0, NUMBER_OF_SECTORS);
	
	/* Read the root dir to get the free blocks list. */
	ds_read_sector(0, (void*)&root_dir, SECTOR_SIZE);
	
	next = root_dir.free_sectors_list;

	while(next){
		/* The sector is in the free list, mark with 1. */
		sector_array[next] = 1;
		
		/* move to the next free sector. */
		ds_read_sector(next, (void*)&sector, SECTOR_SIZE);
		
		next = sector.next_sector;
		
		free_space += SECTOR_SIZE;
	}

	/* Create a log file. */
	if( (log = fopen(log_f, "w")) == NULL){
		perror("fopen()");
		free(sector_array);
		ds_stop();
		return 1;
	}
	
	/* Write the the sector map to the log file. */
	for(i=0;i<NUMBER_OF_SECTORS;i++){
		if(i%32==0) fprintf(log, "%s", "\n");
		fprintf(log, " %d", sector_array[i]);
	}
	
	fclose(log);
	
	/* Execute gnuplot to generate the sector's free map. */
	pid = fork();
	if(pid==0){
		execvp("gnuplot", exec_params);
	}
	
	wait(&status);
	
	free(sector_array);
	
	ds_stop();
	
	printf("Free space %d kbytes.\n", free_space/1024);
	
	return 0;
}

