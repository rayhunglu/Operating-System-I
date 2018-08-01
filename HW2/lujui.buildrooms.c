#include <dirent.h>
#include <pwd.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAXCONNECT 6
#define MINCONNECT 3
#define NUM_ROOMS 7

const char *room_names[NUM_ROOMS] = {
    "room1",
    "room2",
    "room3",
    "room4",
    "room5",
    "room6",
    "room7"
};
struct room {
    int cap_conns;
    char *name;
    enum room_type type;
    unsigned int num_conns;
    struct room *connections[NUM_ROOMS];
};
enum room_type {
    START_ROOM,
    END_ROOM,
    MID_ROOM
};
struct room roomlist[NUM_ROOMS];

struct room *buildroom(){
    int i,j;
    int checkroomset[NUM_ROOMS];
    for(i=0,i<NUM_ROOMS;i++){
        roomlist[i].num_conns=0;
        int con=rand()%(NUM_ROOMS-MINCONNECT);
        roomlist[i].cap_conns=con+3;
        while(1){
            int name= rand()%NUM_ROOMS;
            if(!checkroomset[name]){
                checkroomset[name]=1;
                roomlist[i].name=room_names[name];
            }
        }
        roomlist[i].type=MID_ROOM;
    }
    for(i=0,i<NUM_ROOMS;i++){
        for (j=0;j<roomlist[i].cap_conns;j++){
            int random_room = rand()%NUM_ROOMS;
            while (!checkconnect(i,random_room,roomlist)){
                random_room = rand() % NUM_ROOMS;
            }
        }
    }
    roomlist[0].type = START_ROOM;
    roomlist[NUM_ROOMS - 1].type = END_ROOM;
    return roomlist;
}
int checkconnect(int room_A,int room_B,struct room roomlist[NUM_ROOMS]){
    //A->B
    struct room *roomA=roomlist[room_A];
    struct room *roomB=roomlist[room_B];
    
    if(roomB->num_conns>=MAXCONNECT){
        return 0;
    }
    if(alreadyconnect(roomA,roomB)){
        return 0;
    }
    if(roomA->num_conns==MAXCONNECT){
        return 1;
    }
    roomA->connections[roomA->num_conns]=roomB;
    roomB->connections[roomB->num_conns]=roomA;
    roomA->num_conns++;
    roomB->num_conns++;
    return 1;
}
int alreadyconnect(struct room *roomA,struct room *roomB){//???
    if(roomA==roomB){
        return 1;
    }
    for(int i=0;i<roomA->num_conns;i++){
        if(roomA->connections[i]==roomb&&roomA->connections[i]!=NULL){
            return 1;
        }
    }
    return 0;
}
void buildroomdir(struct room roomlist[]){
    int pid = getpid();
    int uid = getuid();
    struct passwd *user_info = getpwuid(uid);
    long buffer_max_len = strlen(".rooms.") + strlen(user_info->pw_name) + 10;
    char *dir_name = malloc(buffer_max_len * sizeof(char));
    assert(dir_name != NULL);
    sprintf(dir_name, "%s.rooms.%d", user_info->pw_name, pid);
    
    mkdir(dir_name, 0777);
    chdir(dir_name);
    for (int i = 0; i < NUM_ROOMS; i++) {
        FILE *fp = fopen(roomlist[i].name, "w");
        fprintf(fp, "ROOM NAME: %s\n", roomlist[i].name);
        for (int j = 0; j < roomlist[i].num_conns; j++) {
            fprintf(fp, "CONNECTION %d: %s\n", j + 1, roomlist[i].CONNECTION[j]->name);//???
        }
    
        if(roomlist[i].type==START_ROOM){
            fprintf(fp, "ROOM TYPE: START_ROOM");
        }
        else if(roomlist[i].type==END_ROOM){
            fprintf(fp, "ROOM TYPE: END_ROOM");
        }
        else{
            fprintf(fp, "ROOM TYPE: MID_ROOM");
        }
        fclose(fp);
    }
    chdir("..");
    free(dir_name);
}
int main() {
    srand((unsigned) time(0));
    buildroom();
    buildroomdir(roomlist);
    return 0;
}
