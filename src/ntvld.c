/*
 * (C) 2012 Mario Ricardo Rodriguez Somohano
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>
 *
 * This program is not a daemon actually...
 * Each supernode, node and tunnel instances act as a stand alone deamons, the purpose of ntvld is automate the demonize process based on a config file
 *
 * Called from /etc/init.d/ntvl
 * Compile: gcc minIni.c ntvld.c -o ntvld
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include "minIni.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/errno.h>

typedef enum { false, true } bool;
enum { STR16 = 16, STR64 = 64, STR255 = 255 };

const char* program_name;
const char* program_version;
char* config_filename = NULL;
char* log_file=NULL;
int verbose=0;
int errflag=0;


/* node structure definition */
typedef struct {
	bool  CONFIGURED;
	char SUPERNODE[STR255];
	char NETNAME[STR255];
	char SECRET[STR255];
	char DEVICE[STR16];
	char NETWORK[STR16];
	char NETMASK[STR16];
	char ADDRESS[STR16];
	char BROADCAST[STR16];
	char GATEWAY[STR16];
} node_config;

const int MAX_NODES=4;
node_config nodes[4];

/* tunnel structure definition */
typedef struct {
	bool  CONFIGURED;
	int  LPORT;
	char REMOTE[STR255];
	int  RPORT;
	char COMMAND[STR255];
} tunnel_config;

const int MAX_TUNNELS=4;
tunnel_config tunnels[4];


char NTVL_LOGPATH[STR255] = "/var/log/ntvl";
char NTVL_LOGFILE[STR255] = "ntvl.log";
char NTVL_RUNPATH[STR255] = "/var/run/ntvl";
char NTVL_EXECPATH[STR255]= "/usr/sbin";
char NTVL_TUNCTLPATH[STR255]= "/usr/sbin";
char NTVL_ALLOW[STR255] = "/etc/ntvl/ntvl.allow";
char NTVL_DENY[STR255] = "/etc/ntvl/ntvl.deny";
int NTVL_ENABLE_SUPERNODE = 0;
int NTVL_ENABLE_TUNNELS = 0;
int NTVL_ENABLE_SSH_TUNNELS = 0;
int NTVL_SUPERNODE_MANAGEMENT_PORT = 1970;
int NTVL_SUPERNODE_PORT = 1971;
char NTVL_SUPERNODE_FQDN[STR255] = "localhost";
char NTVL_NETNAME[STR255] = "localnet";
char NTVL_SECRET_WORD[STR255] = "unknown";
char NTVL_DEVICE[STR16] = "tap";
char NTVL_NETWORK[STR16]="169.254.1.0";
char NTVL_NETMASK[STR16]="255.255.255.0";
char NTVL_ADDRESS[STR16]="169.254.1.1";
char NTVL_BROADCAST[STR16]="169.254.1.255";
char NTVL_GATEWAY[STR16]="169.254.1.1";
int NTVL_TUNNEL_LPORT=2100;
int NTVL_TUNNEL_RPORT=2101;
char NTVL_REMOTE[STR255]="169.254.1.1";
char NTVL_COMMAND[STR255]="";

/* ************************************************************************ */
static void print_version (FILE* stream) {
  fprintf (stream, "%s v%s\n\n", program_name, program_version);
}

/* ************************************************************************ */
static void print_usage (FILE* stream, int exit_code) {
  print_version(stream);
  fprintf (stream, "Usage: ntvld [-vskt] [-c filename] [-h] \n");
  fprintf (stream,
           "  -h  --help			Display this usage information.\n"
           "  -c  --config filename		Use alternate config file.\n"
		   "  -s  --start			Start daemons.\n"
		   "  -k  --stop			Stop daemons.\n"
		   "  -t  --status			Show status.\n"
           "  -v  --verbose			Run in verbose mode or show version.\n"
		   "\n");
  exit (exit_code);
}

/* ************************************************************************ */
static void throw_error(char *where, char *err, char *what) {
	char message[STR255];
	strcpy(message, where);
	strcat(message, "=> ");
	strcat(message, err);
	strcat(message, "=> ");
	strcat(message, what);
	fprintf(stderr,"%s\n", message);
	errflag=1;
}

/* ************************************************************************ */
static bool valid_ip(char *ipStr) {
	/* check if valid reserved words */
	if (  ( strcmp(ipStr,"dhcp")==0) || (strcmp(ipStr,"localhost")==0)  ) return true;

	/* check if valid IP */
	struct sockaddr_in sa;
	if (inet_pton(AF_INET, ipStr, &(sa.sin_addr))>0) return true;	/* inet_ntop(AF_INET, &(sa.sin_addr), str, INET_ADDRSTRLEN); <= Convert ip & returned in str*/
	else { /* check if valid hostname */
		struct hostent *he;	
		he = gethostbyname (ipStr);
		if (he) return true;
	}
	return false;
}

/* ************************************************************************ */
static char* getFileName(char *path, char *filename, char *dest) {
	if ( ( sizeof(path)+sizeof(filename) ) <= STR255 ) {
		memset(dest, 0, sizeof(dest));
		strcpy(dest, path);
		strcat(dest, "/");
		strcat(dest, filename);
		return dest;
	}
	return NULL;
}
/* ************************************************************************ */
static bool file_exists(char *path, char *filename) {
	char str[STR255];
	getFileName(path, filename, str);
	FILE *file = fopen(str, "r");
    if (file !=NULL) {
        fclose(file);
        return true;
    }
    return false;
}

/* ************************************************************************ */
/* readConfig */
/* ************************************************************************ */
static bool read_config() {

#define sizearray(a)  (sizeof(a) / sizeof((a)[0]))

	long n;
	char section[STR64];
	char str[STR255];
	int i,j;
	
	/* check if config file exists */
	if (config_filename==NULL) config_filename="/etc/ntvl/ntvl.conf";

	FILE *fhc=fopen(config_filename, "r");
	if ( fhc != NULL ) fclose (fhc);
	else {
		perror("Error opening the config file, using defaults");
		errflag=1;
	}
	
	/* prepare log file */
	n = ini_gets("general", "logpath", NTVL_LOGPATH, str, sizearray(str), config_filename);		if (n>0) strcpy(NTVL_LOGPATH, str);
	n = ini_gets("general", "logfile", NTVL_LOGFILE, str, sizearray(str), config_filename);		if (n>0) strcpy(NTVL_LOGFILE, str);

	log_file=getFileName(NTVL_LOGPATH,NTVL_LOGFILE,str);
	
	FILE *fhl=fopen(log_file, "a+");
	if ( fhl != NULL ) fprintf(fhl,"\n[ntvld config start]\n"); 
	else {
		perror("Error opening the log file");
		errflag=1;
	}
	
	/* get general config values */
	if (verbose && fhl!=NULL) fprintf(fhl,"***** general & supernode ******\n");

	n = ini_gets("general", "runpath", NTVL_RUNPATH, str, sizearray(str), config_filename);		if (n>0) strcpy(NTVL_RUNPATH,str);			if (verbose && fhl!=NULL) fprintf(fhl,"Runpath: %s\n", NTVL_RUNPATH);
	n = ini_gets("general", "execpath", NTVL_EXECPATH, str, sizearray(str), config_filename);	if (n>0) strcpy(NTVL_EXECPATH,str);			if (verbose && fhl!=NULL) fprintf(fhl,"Execpath: %s\n", NTVL_EXECPATH);
	n = ini_gets("general", "tunctlpath", NTVL_TUNCTLPATH, str, sizearray(str), config_filename);if (n>0) strcpy(NTVL_EXECPATH,str);		if (verbose && fhl!=NULL) fprintf(fhl,"Tunctl path: %s\n", NTVL_TUNCTLPATH);
	n = ini_gets("general", "allowfile", NTVL_ALLOW, str, sizearray(str), config_filename);		if (n>0) strcpy(NTVL_ALLOW, str);			if (verbose && fhl!=NULL) fprintf(fhl,"Allow file: %s\n", NTVL_ALLOW);
	n = ini_gets("general", "denyfile", NTVL_DENY, str, sizearray(str), config_filename);		if (n>0) strcpy(NTVL_DENY, str);			if (verbose && fhl!=NULL) fprintf(fhl,"Deny file: %s\n", NTVL_DENY);
	n = ini_getl("general", "enable_supernode", NTVL_ENABLE_SUPERNODE, config_filename);		if (n>0) NTVL_ENABLE_SUPERNODE=1;			if (verbose && fhl!=NULL) fprintf(fhl,"Enable supernode: %d\n", NTVL_ENABLE_SUPERNODE);
	n = ini_getl("general", "enable_tunnels", NTVL_ENABLE_TUNNELS, config_filename);			if (n>0) NTVL_ENABLE_TUNNELS=1;				if (verbose && fhl!=NULL) fprintf(fhl,"Enable tunnels: %d\n", NTVL_ENABLE_TUNNELS);
	n = ini_getl("general", "enable_ssh_tunnels", NTVL_ENABLE_SSH_TUNNELS, config_filename);	if (n>0) NTVL_ENABLE_SSH_TUNNELS=1;			if (verbose && fhl!=NULL) fprintf(fhl,"Enable ssh tunnels: %d\n", NTVL_ENABLE_SSH_TUNNELS);
	n = ini_getl("supernode", "mport", NTVL_SUPERNODE_MANAGEMENT_PORT, config_filename);		if (n>0) NTVL_SUPERNODE_MANAGEMENT_PORT=n;	if (verbose && fhl!=NULL) fprintf(fhl,"Supernode management port: %d\n", NTVL_SUPERNODE_MANAGEMENT_PORT);
	n = ini_getl("supernode", "port", NTVL_SUPERNODE_PORT, config_filename);					if (n>0) NTVL_SUPERNODE_PORT=n;				if (verbose && fhl!=NULL) fprintf(fhl,"Supernode port: %d\n", NTVL_SUPERNODE_PORT);

	/* check for executable files */
	if (file_exists(NTVL_EXECPATH,"supernode")==false) throw_error("general","supernode executable not found at",NTVL_EXECPATH);
	if (file_exists(NTVL_EXECPATH,"node")==false) throw_error("general","node executable not found at",NTVL_EXECPATH);
	if (file_exists(NTVL_EXECPATH,"tunnel")==false)	throw_error("general","tunnel executable not found at",NTVL_EXECPATH);
	if (file_exists(NTVL_TUNCTLPATH,"tunctl")==false)	throw_error("general","tunctl executable not found at",NTVL_TUNCTLPATH);
	
	/* get nodes configuration */
	i=0; j=0;
	while (ini_getsection(j, section, sizearray(section), config_filename)>0) {
		char *found = strstr(section, "node:");
		if (found !=NULL) {
			char nodesection[10];
			char auxStr[15];
			char digit[2];
			strcpy(nodesection,"node:");
			strcpy(digit,"0");
			digit[0]=i+49;
			digit[1]=0;
			strcat (nodesection, digit);

			n = ini_gets(nodesection,"supernode", NTVL_SUPERNODE_FQDN, str, sizearray(str), config_filename);	if (n>0) strcpy(NTVL_SUPERNODE_FQDN, str); 	if (valid_ip(NTVL_SUPERNODE_FQDN)==false) throw_error(section,"invalid supernode IP address or DNS failure",NTVL_NETWORK);
			n = ini_gets(nodesection,"netname", NTVL_NETNAME, str, sizearray(str), config_filename); 			if (n>0) strcpy(NTVL_NETNAME, str);
			n = ini_gets(nodesection,"secret", NTVL_SECRET_WORD, str, sizearray(str), config_filename);			if (n>0) strcpy(NTVL_SECRET_WORD, str);
			strcpy(auxStr, NTVL_DEVICE);
			digit[0]=i+48;
			strcat (auxStr, digit);
			n = ini_gets(nodesection,"device", auxStr, str, sizearray(str), config_filename);					if (n>0) strcpy(NTVL_DEVICE, str);
			n = ini_gets(nodesection,"network", NTVL_NETWORK, str, sizearray(str), config_filename);			if (n>0) strcpy(NTVL_NETWORK, str); 	if (valid_ip(NTVL_NETWORK)==false) throw_error(section,"invalid network IP address",NTVL_NETWORK);
			n = ini_gets(nodesection,"netmask", NTVL_NETMASK, str, sizearray(str), config_filename);			if (n>0) strcpy(NTVL_NETMASK, str); 	if (valid_ip(NTVL_NETMASK)==false) throw_error(section,"invalid netmask IP address",NTVL_NETMASK);
			strcpy(NTVL_ADDRESS, "169.254.1.");
			strcpy(auxStr, NTVL_ADDRESS);
			digit[0]=i+49;
			strcat (auxStr, digit);
			n = ini_gets(nodesection,"address", auxStr, str, sizearray(str), config_filename);					if (n>0) strcpy(NTVL_ADDRESS, str); 	if (valid_ip(NTVL_ADDRESS)==false) throw_error(section,"invalid IP address",NTVL_ADDRESS);
			n = ini_gets(nodesection,"broadcast", NTVL_BROADCAST, str, sizearray(str), config_filename);		if (n>0) strcpy(NTVL_BROADCAST, str); 	if (valid_ip(NTVL_BROADCAST)==false) throw_error(section,"invalid broadcast IP address",NTVL_BROADCAST);
			n = ini_gets(nodesection,"gateway", NTVL_GATEWAY, str, sizearray(str), config_filename);			if (n>0) strcpy(NTVL_GATEWAY, str); 	if (valid_ip(NTVL_GATEWAY)==false) throw_error(section,"invalid gateway IP address or DNS failure",NTVL_GATEWAY);
			nodes[i].CONFIGURED=true;
			strcpy(nodes[i].SUPERNODE, NTVL_SUPERNODE_FQDN);
			strcpy(nodes[i].NETNAME, NTVL_NETNAME);
			strcpy(nodes[i].SECRET, NTVL_SECRET_WORD);
			strcpy(nodes[i].DEVICE, NTVL_DEVICE);
			strcpy(nodes[i].NETWORK, NTVL_NETWORK);
			strcpy(nodes[i].NETMASK, NTVL_NETMASK);
			strcpy(nodes[i].ADDRESS, NTVL_ADDRESS);
			strcpy(nodes[i].BROADCAST, NTVL_BROADCAST);
			strcpy(nodes[i].GATEWAY, NTVL_GATEWAY);
			i++;
		}
		j++;
	}
	
	/* get tunnels configuration */
	i=0; j=0;
	while (ini_getsection(j, section, sizearray(section), config_filename)>0) {
		char *found = strstr(section, "tunnel:");
		if (found !=NULL) {
			char tunnelsection[STR16];
			char digit[2];
			strcpy(tunnelsection,"tunnel:");
			strcpy(digit,"0");
			digit[0]=i+49;
			digit[1]=0;
			strcat (tunnelsection, digit);

			n = ini_getl(tunnelsection, "local-port", NTVL_TUNNEL_LPORT, config_filename);							if (n>0) NTVL_TUNNEL_LPORT=n;
			n = ini_gets(tunnelsection, "remote", NTVL_REMOTE, str, sizearray(str), config_filename);				if (n>0) strcpy(NTVL_REMOTE, str); 		if (valid_ip(NTVL_REMOTE)==false) throw_error(section,"invalid local remote IP address or DNS failure",NTVL_REMOTE);
			n = ini_getl(tunnelsection, "remote-port", NTVL_TUNNEL_RPORT, config_filename);							if (n>0) NTVL_TUNNEL_RPORT=n;
			n = ini_gets(tunnelsection, "command", NTVL_COMMAND, str, sizearray(str), config_filename);				if (n>0) strcpy(NTVL_COMMAND, str);

			/* check for command file */
			if ( sizeof (NTVL_COMMAND)>2 ) if (file_exists(NTVL_EXECPATH,"supernode")==false) throw_error("tunnel","tunnel command not found at",NTVL_COMMAND);

			tunnels[i].CONFIGURED=true;
			tunnels[i].LPORT=NTVL_TUNNEL_LPORT;
			strcpy(tunnels[i].REMOTE, NTVL_REMOTE);
			tunnels[i].RPORT=NTVL_TUNNEL_RPORT;
			strcpy(tunnels[i].COMMAND, NTVL_COMMAND);
			i++;
		}
		j++;
	}

					
	/* log node configurations if verbose enabled and log file is OK */
	if (verbose && fhl!=NULL) {
		for (i=0; i<MAX_NODES; i++) {
			if (nodes[i].CONFIGURED==true) {
				fprintf(fhl,"***** node:%d ******\n",(i+1));
				fprintf(fhl,"Supernode: %s\n",nodes[i].SUPERNODE);
				fprintf(fhl,"Network name: %s\n",nodes[i].NETNAME);
				fprintf(fhl,"Secret key: %s\n",nodes[i].SECRET);
				fprintf(fhl,"Device: %s\n",nodes[i].DEVICE);
				fprintf(fhl,"Network: %s\n",nodes[i].NETWORK);
				fprintf(fhl,"Netmask: %s\n",nodes[i].NETMASK);
				fprintf(fhl,"Address: %s\n",nodes[i].ADDRESS);
				fprintf(fhl,"Broadcast: %s\n",nodes[i].BROADCAST);
				fprintf(fhl,"Gateway: %s\n",nodes[i].GATEWAY);
			}
		}
		for (i=0; i<MAX_TUNNELS; i++) {
			if (tunnels[i].CONFIGURED==true) {
				fprintf(fhl,"***** tunnel:%d ******\n",(i+1));
				fprintf(fhl,"Local port: %d\n",tunnels[i].LPORT);
				fprintf(fhl,"Remote ip: %s\n",tunnels[i].REMOTE);
				fprintf(fhl,"Remote port: %d\n",tunnels[i].RPORT);
				fprintf(fhl,"Command: %s\n",tunnels[i].COMMAND);
			}
		}
	}

	if (fhl !=NULL) fclose(fhl);
	
	if (errflag>0) exit (1);
	
	return true;
}
/* ************************************************************************ */
static void write_pid (pid_t ppid, char* exename, char* filename) {
	char str[STR255];
	char* run_file=NULL;

	run_file=getFileName(NTVL_RUNPATH, filename, str);

	FILE *fhr=fopen(run_file, "w+");
	if ( fhr != NULL ) {
		if (ppid >0) snprintf(str,6,"%d",(int) ppid);
		else {
			int fd[2];
			pid_t pid;
			
			pipe(fd);
			pid=fork();
			if (pid==0) { /* child */
				close(fd[0]); /* child closes up input side */
				dup2(fd[1],1);
				dup2(fd[1],2);
				char *argv[]={"pgrep","-x",exename, NULL};
				execv("/usr/bin/pgrep", argv);
				exit(127);
			} else { /* parent */
				int status;
				close(fd[1]); /* parent closes up output side */
				wait(&status);
				memset(str, 0, sizeof(str));
				read(fd[0], str, sizeof(str));
			}
		}
		fprintf(fhr,"%s",str);
		fclose (fhr);
	} else {
		perror(run_file);
		exit(1);
	}	
}
/* ************************************************************************ */
static void start_daemons() {
/*	# START
		# TODO: Check and use tcp_wrappers allow & deny
		# TODO: If supernode enabled call supernode
*/
	if (read_config()==true) {
		char str[STR255];
		
		log_file=getFileName(NTVL_LOGPATH, NTVL_LOGFILE, str);
		
		FILE *fhl=fopen(log_file, "a+");
		if ( fhl != NULL ) fprintf(fhl,"\nStarting daemons\n"); 
		else perror("Error opening the log file");
		
		pid_t pid;
		
		/* ntvld.pid */	
		pid=getpid();
		if ( fhl != NULL ) fprintf(fhl,"Storing ntvld pid\n"); 
		write_pid(pid, "ntvld", "ntvld.pid");

		/* *************************************************** */
		/* SUPERNODE */
		if (NTVL_ENABLE_SUPERNODE) {
			char command[STR255];
			pid=fork();
			if (pid==0) { /* child process */
				char portStr[6];
				snprintf(portStr,6,"%d",NTVL_SUPERNODE_PORT);
				char *argv[]={"supernode","-l", portStr, NULL}; /* TODO: pass verbose mode */
				strcpy(command,NTVL_EXECPATH);
				strcat(command,"/supernode");
				execv(command,argv);
				exit(127);
			} else { /* pid!=0; parent process */
				write_pid((pid_t)0,"supernode","supernode.pid");
				if ( fhl != NULL ) fprintf(fhl,"executed: %s daemon\n", command); 
			}
		}
		
		/* NODES */
		int i;
		for (i=0; i<MAX_NODES; i++) {
			if (nodes[i].CONFIGURED==true) {
				char command[STR255];
				char nodespid[STR16];
				strcpy(nodespid,"nodes.pid");
				/* *************************************************** */
				/* tunctl */
				pid=fork();
				if (pid==0) { /* child process */
					char *argv[]={"tunctl","-t",nodes[i].DEVICE, NULL};
					strcpy(command,NTVL_TUNCTLPATH);
					strcat(command,"/tunctl");
					execv(command,argv);
					exit(127);
				} else { /* pid!=0; parent process */
					int status;
					wait(&status);
					if ( fhl != NULL ) fprintf(fhl,"executed: %s with result=%d\n", command, status); 
				}
				/* *************************************************** */
				/* ifconfig <device> up */
				pid=fork();
				if (pid==0) { /* child process */
					char *argv[]={"ifconfig",nodes[i].DEVICE, nodes[i].ADDRESS, NULL};
					strcpy(command,"/sbin");
					strcat(command,"/ifconfig");
					execv(command,argv);
					exit(127);
				} else { /* pid!=0; parent process */
					int status;
					wait(&status);					
					if ( fhl != NULL ) fprintf(fhl,"executed: %s with result=%d\n", command, status); 
				}
				/* *************************************************** */
				/* node[i] run and pid */
				pid=fork();
				if (pid==0) { /* child process */
					char portStr[6];
					snprintf(portStr,6,"%d",NTVL_SUPERNODE_PORT);
					strcpy(str,nodes[i].SUPERNODE);
					strcat(str,":");
					strcat(str, portStr);
					char *argv[]={"node","-d",nodes[i].DEVICE, "-c",nodes[i].NETNAME, "-k",nodes[i].SECRET, "-a",nodes[i].ADDRESS, "-l", str, NULL};
					strcpy(command,NTVL_EXECPATH);
					strcat(command,"/node");
					execv(command,argv);
					exit(127);
				} else { /* pid!=0; parent process */
					write_pid((pid_t) 0, "node", nodespid);
					if ( fhl != NULL ) fprintf(fhl,"executed: %s daemon\n", command); 
				}				
			}
		}

		/* TUNNELS */
		for (i=0; i<MAX_TUNNELS; i++) {
			if (tunnels[i].CONFIGURED==true) {
				if (sizeof (tunnels[i].COMMAND)>2) {
					char command[STR255];
					char portStr[6];
					char argument[STR255];
					char tunnelspid[STR16];
					strcpy(tunnelspid,"tunnels.pid");
					snprintf(portStr,6,"%d",tunnels[i].LPORT);
					strcpy(argument,portStr);
					strcat(argument,":");
					strcat(argument,tunnels[i].REMOTE);
					strcat(argument,":");
					snprintf(portStr,6,"%d",tunnels[i].RPORT);
					strcat(argument, portStr);
					/* tunnel[i] run and pid */
					pid=fork();
					if (pid==0) { /* child process */
						char *argv[]={"tunnel",argument,"--cmd",tunnels[i].COMMAND, NULL};
						strcpy(command,NTVL_EXECPATH);
						strcat(command,"/tunnel");
						/* execv(command,argv); */
						exit(127);
					} else { /* pid!=0; parent process */
						write_pid((pid_t) 0, "tunnel", tunnelspid);
						if ( fhl != NULL ) fprintf(fhl,"executed: %s daemon\n", command); 
					}
				} else throw_error("start_daemons","no command to process", "tunnel");
			}
		}

		
	} else {
		throw_error("start_daemons","can't read file", "config");
		exit(1);
	}
}

/* ************************************************************************ */
static int kill_instances(char *procname) {
	char* pid_file=NULL;
	int ipid=0, killreturn=-1;
	char str[STR255];
	char aux[STR255];
	int numproc=0;;

	strcpy(str,procname);
	strcat(str,".pid");
	pid_file=getFileName(NTVL_RUNPATH,str,aux);
	
	FILE *fhp=fopen(pid_file, "r");
	if (fhp != NULL) {
		while ( fgets ( str, sizeof str, fhp ) != NULL ) {
			ipid=atoi(str);
			fprintf(stdout,"Stopping %s [%d]...", procname, ipid);
			killreturn=killpg((pid_t)ipid,SIGKILL);
			strcpy(str,"OK");
			if (killreturn==-1) strcpy(str,"Not running");
			if (killreturn==EPERM ) strcpy(str,"Permission denied");
			fprintf(stdout,"%s [%d]\n", str, killreturn);
			numproc++;
		}
		fclose(fhp);
	}
	if (killreturn<=0) remove(pid_file);
	return numproc;
}
/* ************************************************************************ */
static void stop_daemons() {
	if (read_config()==true) {
		char str[STR255];
		int cant=0;

		log_file=getFileName(NTVL_LOGPATH,NTVL_LOGFILE,str);
		FILE *fhl=fopen(log_file, "a+");
		if ( fhl != NULL ) fprintf(fhl,"\nShowing status\n"); 
		else perror("Error opening the log file");

		cant+=kill_instances("ntvld");
		cant+=kill_instances("supernode");
		cant+=kill_instances("nodes");
		cant+=kill_instances("tunnels");
		
		if (cant==0) fprintf(stdout, "No daemons running\n");		
	}
}
/* ************************************************************************ */
static int show_instances(char *procname) {
	char* pid_file=NULL;
	char str[STR255];
	char aux[STR255];
	int numproc=0;

	strcpy(str,procname);
	strcat(str,".pid");
	pid_file=getFileName(NTVL_RUNPATH,str,aux);
	
	FILE *fhp=fopen(pid_file, "r");
	if (fhp != NULL) {
		while ( fgets ( str, sizeof str, fhp ) != NULL ) {
			fprintf(stdout,"Instance of %s running with pid=%s", procname, str);
			numproc++;
		}
		fprintf(stdout,"\n");
		fclose(fhp);
	}
	return numproc;
}
/* ************************************************************************ */

static void show_status() {
	if (read_config()==true) {
		int cant=0;

		char str[STR255];
		log_file=getFileName(NTVL_LOGPATH,NTVL_LOGFILE,str);
		FILE *fhl=fopen(log_file, "a+");
		if ( fhl != NULL ) fprintf(fhl,"\nShowing status\n"); 
		else perror("Error opening the log file");
		
		cant+=show_instances("ntvld");
		cant+=show_instances("supernode");
		cant+=show_instances("nodes");
		cant+=show_instances("tunnels");
		
		if (cant==0) fprintf(stdout, "No daemons running\n");
	}
}
/* ************************************************************************ */
/* main */
/* ************************************************************************ */
int main (int argc, char* argv[]) {
  program_name = argv[0];
  program_version="1.0.0";
  
  int opt;
  int selected_process=0;

  /* A string listing valid short options letters.  */
  const char* const short_options = "hc:sktv";
  /* An array describing valid long options.  */
  const struct option long_options[] = {
    { "help",		0, NULL, 'h' },
    { "config",		1, NULL, 'c' },
    { "start",		0, NULL, 's' },
    { "stop",		0, NULL, 'k' },
    { "ststus",		0, NULL, 't' },
    { "verbose",	0, NULL, 'v' },
    { NULL,			0, NULL, 0   }   /* Required at end of array.  */
  };


  do {
	opt = getopt_long (argc, argv, short_options, long_options, NULL);
	switch (opt) {
		case 'h':   /* -h or --help */
			print_usage (stdout, 0);

		case 'c':   /* -c or --config */
			/* This option takes an argument, the name of the config file.  */
			config_filename = optarg;
			break;
	  
		case 's': 	/* -s or --start */
			if (selected_process=='k') selected_process='?';
			else selected_process=opt;
			break;

		case 'k': 	/* -k or --stop */
			if (selected_process=='s') selected_process='?';
			else selected_process=opt;
			break;

		case 't': 	/* -t or --status */
			selected_process=opt;
			break;

		case 'v':   /* -v or --version */
			verbose=1;
			if (!selected_process) selected_process=opt;
			break;

		case '?':   /* The user specified an invalid option.  */
			/* Print usage information to standard error, and exit with exit code one (indicating abnormal termination).  */
			print_usage (stderr, 1);

		case -1:    /* Done with options.  */
			break;

		default:    /* Something else: unexpected.  */
			abort ();
    }
  }

	while (opt != -1);

  /* Done with options.  OPTIND points to first non-option argument. */
	if (verbose) {
		int i;
		for (i = optind; i < argc; ++i) printf ("%s: unknown Argument: %s\n", program_name, argv[i]);
	}

	switch (selected_process) {
		case 's': start_daemons();	break;
		case 'k': stop_daemons();	break;
		case 't': show_status();	break;
		case 'v': print_version(stdout); break;
		default: fprintf(stdout, "%s nothing to do\n\n", program_name);
	}
	
  return 0;
}
