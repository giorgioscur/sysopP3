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
	int ret, tam, tamaux=0, i=0,k,NS,j,dirAddress;
    struct root_table_directory root_dir;
	struct sector_data sector;
	struct table_directory directory = {0}, newDir  = {0};
	char *data;
	data = malloc (508 * sizeof (char));
	char name[50];
	char * str = (char *) malloc(100);
	strcpy(str,simul_file);
	char* pch = strtok(simul_file, "/");
    FILE *ptr;

	if ( (ret = ds_init(FILENAME, SECTOR_SIZE, NUMBER_OF_SECTORS, 0)) != 0 ){
		return ret;
	}
    
    ds_read_sector(0,(void*)&root_dir, SECTOR_SIZE);

    ptr = fopen (input_file,"rb");

	//Verifica se deve escrever no root ou não
	while(pch != NULL) { 
		pch =  strtok(NULL, "/");
		i++;
	}

    
    // Captura o tamanho do arquivo

  	ftell(ptr);
	fseek(ptr, 0L, SEEK_END);
	tam = ftell(ptr);
	fseek(ptr, 0, SEEK_SET);
			
	// Pega o nome do arquivo a ser criado
		get_name(name, str);

	// Preenche o primeiro bloco
	printf("\n a %d", i);
	if(i == 1){
		for (j = 0; j < 15; j++){
			if (root_dir.entries[j].sector_start == 0){
				root_dir.entries[j].dir = 0;
				strcpy(root_dir.entries[j].name, name);
				root_dir.entries[j].size_bytes = tam; 
				root_dir.entries[j].sector_start = root_dir.free_sectors_list;
				NS = root_dir.free_sectors_list;
				ds_read_sector(NS,(void*)&sector, SECTOR_SIZE);
				break;
			}	
		}		
	} else {
		dirAddress = change_directory(str);
		ds_read_sector(dirAddress,(void*)&directory, SECTOR_SIZE);
		for (j = 0; j < 15; j++){
			if(directory.entries[j].sector_start == 0) {
				directory.entries[j].dir = 0;
				strcpy(directory.entries[j].name, name);
				directory.entries[j].size_bytes = tam;
				directory.entries[j].sector_start = root_dir.free_sectors_list;
				
				ds_write_sector(dirAddress, (void*)&directory, SECTOR_SIZE); 
				ds_write_sector(directory.entries[j].sector_start, (void*)&newDir, SECTOR_SIZE);
				
				NS = root_dir.free_sectors_list;
				ds_read_sector(NS,(void*)&sector, SECTOR_SIZE);
				break;
			}
		}
	}
	printf("\nteste\n");
	int falta_ler, sera_lido, ANS, NNS;
	falta_ler = tam;	
	
    if(tam > 508){
    	while(tamaux < tam){   
			if(falta_ler > 508)
	 			sera_lido = 508;
			else
				sera_lido = falta_ler;
				
			char *data2;
			data2 = malloc (sera_lido * sizeof (char));

			fseek(ptr, tamaux, SEEK_SET);
			fread(data2, sizeof(char), sera_lido ,ptr);		
			for(k=0; k<508; k++)
				sector.data[k] = data2[k];	
			sector.next_sector = ANS;
			//ds_read_sector(NS,(void*)&sector, SECTOR_SIZE);

			//  ANS = next_free_sector();
			 printf("\n NS: %d", NS);
			ds_write_sector(NS,(void*)&sector, SECTOR_SIZE);
			NNS = NS;
			ANS = NS+1;
			printf("\nANS: %d",ANS);
			ds_read_sector(ANS,(void*)&sector, SECTOR_SIZE);
			NS = ANS;
			falta_ler = falta_ler - 508;
			tamaux = tamaux + 508;
       }
	   	printf("\nNNS: %d", NNS);
		 ds_read_sector(NNS,(void*)&sector, SECTOR_SIZE);
		 sector.next_sector = 0;
		 ds_write_sector(NNS,(void*)&sector, SECTOR_SIZE);
     }
	 root_dir.free_sectors_list = NS;
	ds_write_sector(0,(void*)&root_dir, SECTOR_SIZE);

	fclose(ptr);
    printf("\n %d \n", root_dir.free_sectors_list);
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
	int ret, testa, i,j=0, tamanho, tam2, dirAddress, B=0, ind;
	char name[50];
	struct root_table_directory root_dir;
	struct sector_data sector;
	struct table_directory directory;
	char* pch = strtok(simul_file, "/");
	FILE *fp;

	char *data;
	data = malloc (508 * sizeof (char));
	if ( (ret = ds_init(FILENAME, SECTOR_SIZE, NUMBER_OF_SECTORS, 0)) != 0 ){
		return ret;
	}

	while(pch != NULL) { 
		pch =  strtok(NULL, "/");
		i++;
	}

	ds_read_sector(0,(void*)&root_dir, SECTOR_SIZE);
	get_name(name,simul_file);
	printf("\n %s\n", name);

	fp = fopen (output_file,"wb");
	if(i == 1){
		printf("\n teste \n");
		for(j=1; j<15; j++)
		{
			testa = strcmp(root_dir.entries[j].name,name);
			printf("\nname: %s testa %d\n",root_dir.entries[j].name, testa);
			if(testa == 0){
				ind = j;
				while(B==0)
				ds_read_sector(ind,(void*)&sector, SECTOR_SIZE);		
				printf("\n ACHEI \n");
				strcpy(data,sector.data);				
				printf("\n %s  \n", data);
				fwrite(data,sizeof(char),508,fp);
				memset(data, 0, 508);
				if(sector.next_sector == 0)
					B = 1;
				ind = sector.next_sector;
			}
		}
	} else{
		dirAddress = change_directory(simul_file);
		for(j=0; j<15; j++)
		{
		
		}
	}
	
	
	// get_next_bar(name, simul_file,1);
	// printf("\n %s", name);
	// get_next_bar(name, simul_file,2);
	// printf("\n %s", name);
	// get_next_bar(name, simul_file,3);
	// printf("\n %s", name);
	// ds_read_sector(0,(void*)&root_dir, SECTOR_SIZE);
	
	// while(achou == 0){
	// //	if(name)
	// achou =1;
	// }
	fclose(fp);
	ds_stop();
	
	return 0;
}


/**
 * @brief Delete file from file system.
 * @param simul_file Source file path.
 * @return 0 on success.
 */
int fs_del(char* directory_path){ //NAO TESTEI AINDA, mas ta compilando isso q vale
	int ret, i=0, j=0, dirAddress,success, occupiedSector;
	char* nome = (char *) malloc(100);
	char* pch;
	struct root_table_directory root_dir;
	struct table_directory directory;
	struct sector_data sector, sectorDel;
	char * str = (char *) malloc(100);

	if ( (ret = ds_init(FILENAME, SECTOR_SIZE, NUMBER_OF_SECTORS, 0)) != 0 ){
		return ret;
	}
	
	strcpy(str,directory_path);
	pch = strtok(directory_path, "/");

	/* Write the code to delete a directory. */
	ds_read_sector(0,(void*)&root_dir, SECTOR_SIZE); //Read root dir

	while(pch != NULL) { 
		nome = pch;
		pch =  strtok(NULL, "/");
		i++;
	}
	if(i > 1) {

		dirAddress = change_directory(str);
		ds_read_sector(dirAddress,(void*)&directory, SECTOR_SIZE);
		for(i= 0;i<16;i++) {
			if(!strcmp(directory.entries[i].name,nome)) {

				memset(&sector, 0, SECTOR_SIZE);
				ds_read_sector(directory.entries[i].sector_start,(void*)&sectorDel, SECTOR_SIZE); //Le o primeiro da fila a ser deletado
				occupiedSector = directory.entries[i].sector_start; //Guarda o numero do setor do primeiro da fila
				while(sectorDel.next_sector != 0){ // Enquanto o proximo setor nao for o ultimo
					sector.next_sector = root_dir.free_sectors_list;	//Cria o link entre o setor a ser liberado e o antigo free
					root_dir.free_sectors_list = occupiedSector;		//O free_sectors_list vira o o setor a ser liberado
					occupiedSector = sectorDel.next_sector;				//Guarda o endereco do proximo da fila
					ds_write_sector(root_dir.free_sectors_list,(void*)&sector, SECTOR_SIZE); //Escreve o setor vazio, efetivamente apagando o setor
					ds_read_sector(sectorDel.next_sector,(void*)&sectorDel, SECTOR_SIZE);	//Pega o proximo setor ocupado da fila
				}
				sector.next_sector = root_dir.free_sectors_list;	//Link do ultimo setor a ser liberado
				root_dir.free_sectors_list = occupiedSector;		//O free_sectors_list vira o o setor a ser liberado
				ds_write_sector(root_dir.free_sectors_list,(void*)&sector, SECTOR_SIZE); //Escreve o setor vazio, efetivamente apagando o setor
				directory.entries[i].sector_start = 0; // "deleta" da lista do diretorio
				ds_write_sector(dirAddress, (void*)&directory, SECTOR_SIZE); //Salva o diretorio
				success = 1;
				break;
			}
		}

	} else { 
		for(i= 0;i<15;i++) {
			if(!strcmp(root_dir.entries[i].name,nome)) {
				ds_read_sector(root_dir.entries[i].sector_start,(void*)&sectorDel, SECTOR_SIZE);
				occupiedSector = root_dir.entries[i].sector_start; //Guarda o numero do setor do primeiro da fila
				while(sectorDel.next_sector != 0){ // Enquanto o proximo setor nao for o ultimo
					sector.next_sector = root_dir.free_sectors_list;	//Cria o link entre o setor a ser liberado e o antigo free
					root_dir.free_sectors_list = occupiedSector;		//O free_sectors_list vira o o setor a ser liberado
					occupiedSector = sectorDel.next_sector;				//Guarda o endereco do proximo da fila
					ds_write_sector(root_dir.free_sectors_list,(void*)&sector, SECTOR_SIZE); //Escreve o setor vazio, efetivamente apagando o setor
					ds_read_sector(sectorDel.next_sector,(void*)&sectorDel, SECTOR_SIZE);	//Pega o proximo setor ocupado da fila
				}
				sector.next_sector = root_dir.free_sectors_list;	//Link do ultimo setor a ser liberado
				root_dir.free_sectors_list = occupiedSector;		//O free_sectors_list vira o o setor a ser liberado
				ds_write_sector(root_dir.free_sectors_list,(void*)&sector, SECTOR_SIZE); //Escreve o setor vazio, efetivamente apagando o setor
				root_dir.entries[i].sector_start = 0; //"deleta" da lista do diretorio
				success = 1;
				break;
			}
		}
	}

	if(success) { 
		printf("Arquivo excluido com sucesso\n");

	} else {
		printf("Verifique se o caminho fornecido esta correto");
	}

	ds_write_sector(0, (void*)&root_dir, SECTOR_SIZE); // save root dir


	
	ds_stop();
	
	return 0;
}

/**
 * @brief List files from a directory.
 * @param simul_file Source file path.
 * @return 0 on success.
 */
int fs_ls(char *dir_path){
	int ret,i, dirAddress;
	struct root_table_directory root_dir;
	struct table_directory directory;

	if ( (ret = ds_init(FILENAME, SECTOR_SIZE, NUMBER_OF_SECTORS, 0)) != 0 ){
		return ret;
	}
	
	/* Write the code to show files or directories. */
	ds_read_sector(0,(void*)&root_dir, SECTOR_SIZE); //Read root dir

	if(!strcmp(dir_path,"/")) {
		printf("/\n");
		for(i=0;i<14;i++) {
			if(root_dir.entries[i].sector_start != 0) {
				if(root_dir.entries[i].dir) {
					printf("  d %s\n",root_dir.entries[i].name);
				} else {
					printf("  f %s            %u bits\n",root_dir.entries[i].name,root_dir.entries[i].size_bytes);
				}
			}
		}
	}else{ 
		printf("%s\n", dir_path);
		dirAddress = change_directory(dir_path);
		ds_read_sector(dirAddress,(void*)&directory, SECTOR_SIZE);
		for(i=0;i<15;i++) {
			if(directory.entries[i].sector_start != 0) {
				if(directory.entries[i].dir) {
					printf("  d %s\n",directory.entries[i].name);
				} else {
					printf("  f %s           %u bits\n",directory.entries[i].name, directory.entries[i].size_bytes);
				}
			}
		}
	}
	
	ds_stop();

	return 0;
}

/**
 * @brief Create a new directory on the simulated filesystem.
 * @param directory_path directory path.
 * @return 0 on success.
 */
int fs_mkdir(char* directory_path){
	int ret, i = 0, success =0, dirAddress;
	struct root_table_directory root_dir;
	struct table_directory directory = {0}, newDir  = {0};
	char* nome;
	struct sector_data sector;
	char* pch;
	char * str = (char *) malloc(100);


	/* Initialize Virtual Memory */
	if ( (ret = ds_init(FILENAME, SECTOR_SIZE, NUMBER_OF_SECTORS, 0)) != 0 ){
		return ret;
	}
	
	/* Code to create a new directory. */
	ds_read_sector(0,(void*)&root_dir, SECTOR_SIZE); //Read root dir
	
	strcpy(str,directory_path);

	pch = strtok(directory_path, "/");

	while(pch != NULL) { 
		nome = pch;
		pch =  strtok(NULL, "/");
		i++;
	}

	if(i > 1) {
		dirAddress = change_directory(str);
		ds_read_sector(dirAddress,(void*)&directory, SECTOR_SIZE);
		for(i= 0;i<16;i++) {
			if(directory.entries[i].sector_start == 0) {
				directory.entries[i].dir = 1;
				strcpy(directory.entries[i].name, nome);
				directory.entries[i].size_bytes = 0;
				directory.entries[i].sector_start = root_dir.free_sectors_list;
				success = 1;
				ds_read_sector(directory.entries[i].sector_start,(void*)&sector, SECTOR_SIZE);
				root_dir.free_sectors_list = sector.next_sector;
				ds_write_sector(dirAddress, (void*)&directory, SECTOR_SIZE); 
				ds_write_sector(directory.entries[i].sector_start, (void*)&newDir, SECTOR_SIZE);
				break;
			}
		}
	} else {
		for(i= 0;i<15;i++) {
			if(root_dir.entries[i].sector_start == 0) {
				root_dir.entries[i].dir = 1;
				strcpy(root_dir.entries[i].name, nome);
				root_dir.entries[i].size_bytes = 0;
				root_dir.entries[i].sector_start = root_dir.free_sectors_list;
				success = 1;
				ds_read_sector(root_dir.entries[i].sector_start,(void*)&sector, SECTOR_SIZE);
				root_dir.free_sectors_list = sector.next_sector;
				ds_write_sector(root_dir.entries[i].sector_start, (void*)&newDir, SECTOR_SIZE);
				break;
			}
		}
	}

	if(success){
		printf("Diretorio criado com sucesso\n");
	} else {
		printf("Error");
	}

	ds_write_sector(0, (void*)&root_dir, SECTOR_SIZE); // save root dir
	ds_stop();
	
	return 0;
}

int change_directory(char* directory_path) { 
	char* pch = strtok(directory_path, "/");
	struct root_table_directory root_dir;
	struct table_directory directory;
	int i, sectorAddress;

	ds_read_sector(0,(void*)&root_dir, SECTOR_SIZE); //Read root dir

	for(i=0; i<15; i++) {
		if (!strcmp(root_dir.entries[i].name, pch) && root_dir.entries[i].sector_start != 0 && root_dir.entries[i].dir) { 
			ds_read_sector(root_dir.entries[i].sector_start,(void*)&directory,SECTOR_SIZE);
			sectorAddress = root_dir.entries[i].sector_start;
			while( (pch = strtok(NULL, "/")) != NULL) {
				for(i=0; i<16; i++) {
					if((!strcmp(directory.entries[i].name,pch) == 0) && directory.entries[i].sector_start != 0 && directory.entries[i].dir) {
						sectorAddress = directory.entries[i].sector_start;
					}
				}
			}

			return sectorAddress;
		}
	}
	printf("Erro ao achar diretorio, Verifique se o caminho esta certo\n");
	return 0; 
}

int fs_rmdir(char *directory_path){ //// NEED TO TEST AHHHH
	int ret, i=0, j=0,dirAddress,success;
	char* nome = (char *) malloc(100);
	char* pch;
	struct root_table_directory root_dir;
	struct table_directory directory, directoryDel;
	struct sector_data sector;
	char * str = (char *) malloc(100);
	char* fatherDirPath;

	if ( (ret = ds_init(FILENAME, SECTOR_SIZE, NUMBER_OF_SECTORS, 0)) != 0 ){
		return ret;
	}

	strcpy(str,directory_path);

	pch = strtok(directory_path, "/");

	/* Write the code to delete a directory. */
	ds_read_sector(0,(void*)&root_dir, SECTOR_SIZE); //Read root dir

	while(pch != NULL) { 
		nome = pch;
		pch =  strtok(NULL, "/");
		i++;
	}

	if(i > 1) {
		fatherDirPath = (char*)malloc(100);
		strncpy( fatherDirPath, str, (strlen(str) - strlen(nome)) -1 );
		dirAddress = change_directory(fatherDirPath);
		ds_read_sector(dirAddress,(void*)&directory, SECTOR_SIZE);
		for(i= 0;i<16;i++) {
			if(!strcmp(directory.entries[i].name,nome)) {
				ds_read_sector(directory.entries[i].sector_start,(void*)&directoryDel, SECTOR_SIZE);
				for(j= 0;j<16;j++) {
					if(directoryDel.entries[i].sector_start != 0) {
						printf("Diretorio nao esta vazio\n");
						ds_stop();
						return -1;
					}
				}
				memset(&sector, 0, SECTOR_SIZE);
				sector.next_sector = root_dir.free_sectors_list;
				root_dir.free_sectors_list = directory.entries[i].sector_start;
				ds_write_sector(directory.entries[i].sector_start,(void*)&sector, SECTOR_SIZE);
				directory.entries[i].sector_start = 0;
				ds_write_sector(dirAddress, (void*)&directory, SECTOR_SIZE);
				success = 1;
				break;
			}
		}

	} else { 
		for(i= 0;i<15;i++) {
			if(!strcmp(root_dir.entries[i].name,nome)) {
				ds_read_sector(root_dir.entries[i].sector_start,(void*)&directoryDel, SECTOR_SIZE);
				for(j= 0;j<16;j++) {
					if(directoryDel.entries[i].sector_start != 0) {
						printf("Diretorio nao esta vazio\n");
						ds_stop();
						return -1;
					}
				}
				memset(&sector, 0, SECTOR_SIZE);
				sector.next_sector = root_dir.free_sectors_list;
				ds_write_sector(root_dir.entries[i].sector_start,(void*)&sector, SECTOR_SIZE);
				root_dir.free_sectors_list = root_dir.entries[i].sector_start;
				root_dir.entries[i].sector_start = 0;
				success = 1;
				break;
			}
		}
	}

	if(success) { 
		printf("Diretorio excluido com sucesso\n");

	} else {
		printf("Verifique se o caminho fornecido esta correto");
	}

	ds_write_sector(0, (void*)&root_dir, SECTOR_SIZE); // save root dir
	
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

int next_free_sector(){
struct root_table_directory root_dir;
	int achou = 0 ,k =1;
	ds_read_sector(0,(void*)&root_dir, SECTOR_SIZE);
	while(achou == 0){	
		printf("\n\n ra  %d", root_dir.entries[k].sector_start);
		if(root_dir.entries[k].sector_start == 0){
			return k;
		}
		k++;
	}
	return 0;
}

void get_name(char *name, char* simul_file){
	int i=0, barras =0, posicao;
	while(simul_file[i] != NULL){
		if(simul_file[i] == '/'){
			barras ++;
			posicao = i + 1;
		}
		i++;
	}
	printf("\n BARRAS: %d" ,barras);
	if(barras == 0)    /* Tirar e trocar o input_file pelo simul_file , sempre haverá barra no nome */
		posicao = 0;
	i = 0;
	printf("\n BARRAS: %d" ,i);
	while(simul_file[i] != NULL){
		name[i] = simul_file[posicao];
		i++;
		posicao++;
	}

}

int get_next_bar(char *name, char* simul_file, int nm_bars){
	char* ptr;
	char* str;
	char nameaux[50];
	int i = 0, cont = 1;
	str = simul_file;
	// if(nm_bars == 1){
	// 	ptr = strtok(str,"/");
	// 	strcpy(name,ptr);
	// return 1;
	// }
	ptr = strtok(str,"/");
	strcpy(nameaux,ptr);
	strcat(nameaux,"0");
    while (ptr != NULL){
        printf ("%s\n",ptr);
        ptr = strtok (NULL, "/");
		if(ptr != NULL){
			strcat(nameaux,ptr);
			strcat(nameaux,"0");}
		cont++;
    }
	printf("\n b %s",nameaux);
	if(nm_bars == 1)
	{
		for(i=0;i<cont;i++){
			

		}
	}

}

// int set_first(int setor_livre, char* name, int tam) {
// 	struct root_table_directory root_dir;
// 	struct sector_data sector;
// 	char data[508];

// 	root_dir.entries[setor_livre].dir = 0;
//     strcpy(root_dir.entries[setor_livre].name, name);
//     root_dir.entries[setor_livre].size_bytes = tam;  
//     root_dir.entries[setor_livre].sector_start = setor_livre;

// 	strcpy(sector.data,data);
//     ds_write_sector(setor_livre,(void*)&sector, SECTOR_SIZE);
		
// 	ds_write_sector(0,(void*)&root_dir, SECTOR_SIZE);
// 	root_dir.free_sectors_list = next_free_sector();
//     ds_write_sector(0,(void*)&root_dir, SECTOR_SIZE);

// 	printf("\nt %s\n", root_dir.entries[setor_livre].name);
// 	printf("\nt %d\n", root_dir.free_sectors_list);
// 	printf("\nt %d\n", tam);

// 	return root_dir.free_sectors_list;

// }