#define SQLITE_HAS_CODEC
#define WIN
#include "sqlite3.h"
//#include "jWrite.h"
#include "base64.h"

#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>


// https://github.com/utelle/wxsqlite3
// https://github.com/rindeal/SQLite3-Encryption
// https://gist.github.com/akehrer/481a38477dd0518ec0086ac66e38e0e2
// https://wiki.freepascal.org/Creating_bindings_for_C_libraries
// https://www.codeproject.com/Articles/887604/jWrite-A-Really-Simple-JSON-Writer-in-C
// https://github.com/oconnor663/basex_gmp/blob/master/base62.c
//https://www.codeproject.com/KB/cpp/887604/jWrite_Demo.zip
// http://www.cplusplus.com/forum/beginner/66262/     DLL Function returning a string

//SWIG https://stackoverflow.com/questions/706970/can-i-use-a-dll-written-in-c-with-php


// Monoprice Select Mini V2 Outlet
// https://utelle.github.io/

// this demo http://zetcode.com/db/sqlitec/

// https://github.com/utelle/wxsqlite3

// C:\Program Files (x86)\Windows Kits\10\bin\10.0.18362.0\x64>
// signtool sign /a /t http://timestamp.comodoca.com /fd SHA256 c:\MinGW\bin\jrsqlite3.dll c:\MinGW\bin\jrsqlite3.exe

// gcc jr.c base64.c -L. -ljrsqlite3 -ojrsqlite3.exe
// set path=%path%;C:\MinGW\bin

short debug=1;
char *jr_start="---jr-start---\n";
char *jr_end  ="---jr-end---\n";


int callback(void *, int, char **, char **);
long long sqlite_header=0;
time_t  ts;

int main(void) {
    ts = time(NULL);

    char *filename=NULL,*password=NULL,*newpassword=NULL,*queryfile=NULL,*query=NULL;
    short do_example=1,show_only=0,index,c;
    
 // get params https://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html#Example-of-Getopt
    while ((c = getopt (_argc, _argv, "hg:q:f:p:n:deo")) != -1)
    switch (c)
      {
      case 'q':
        query = optarg;
        break;
      case 'g':
        queryfile = optarg;
        break;
      case 'p':
        password = optarg;
        break;
      case 'n':
        newpassword = optarg;
        break;
      case 'f':
        filename = optarg;
        break;
      case 'd':
        debug = 0;
        break;
      case 's':
        show_only = 0;
        break;
      case 'e':
        do_example = 0;
        break;
      case 'h':
        printf("%s help (compilation time: %s %s)\n\t-q query\n\t-g query file (if no -q & no -g wait for stdin)\n\t-p password\n\t-n new password\n\t-f db file name (test.db)\n\t-d debug\n\t-e no example Cars table create\n\t-s show parsed opt only\n\n",_argv[0],__DATE__,__TIME__);
        return 2;
      case '?':
        if (optopt == 'c')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'. Please use %s -h for help\n", optopt,_argv[0]);
        else
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        return 1;
      default:
        abort ();
      }
  filename=filename ? filename : "test.db";
    
   if (!query ) {
      // https://stackoverflow.com/questions/2496668/how-to-read-the-standard-input-into-string-variable-until-eof-in-c
      #define BUF_SIZE 1024000
      char buffer[BUF_SIZE+1];
      //size_t contentSize = 0; // includes NULL
      long long contentSize = 0; // includes NULL
      // Preallocate space.  We could just allocate one char here, 
      //but that wouldn't be efficient. 
      
      FILE *f = queryfile ? fopen(queryfile, "r") : stdin;
      if (f == NULL)
      {
        fprintf (stderr,"Could not create tmp_file %s",queryfile);
        return 1;
      }
      char *content = malloc(sizeof(char) * BUF_SIZE +1);
      if(content == NULL)
      {
          perror("Failed to allocate content");
          exit(1);
      }
      content[0] = '\0'; // make null-terminated
      //while(fgets(buffer, 1024000, f))
      do {
          size_t bytesRead = fread(buffer, 1, BUF_SIZE,f);
          buffer[bytesRead]=0;
          char *old = content;
          contentSize += bytesRead;
          content = realloc(content, contentSize);
          if(content == NULL)
          {
              perror("Failed to reallocate content");
              free(old);
              exit(2);
          }
          memcpy(content + contentSize - bytesRead, buffer,bytesRead);
          content[contentSize]=0;
//          fprintf(stderr, "%p to %p src %p content:%d read:%d\n%s", content,content + contentSize - bytesRead,buffer, contentSize,bytesRead,content);
          
//          fprintf(stderr, "size %d\n", contentSize);
//break;
      } while (!feof(f));

      if(ferror(f))
      {
          free(content);
          perror("Error reading from stdin.");
          exit(3);
      }
      fclose(f);
//      content[contentSize]=0;
      query=content;
//      fprintf(stderr, "LOADED %s %d %d\n\%s", queryfile,contentSize,strlen(query),query);     
//      exit(4);
  }
  
    
  if (!query) {
      perror("Query not defined.");
      exit(3);
  }
  
  

  if (debug || show_only ) {
    fprintf (stdout,"file  = %s\npass  = %s\nnewpa = %s\ndebug = %d\nquery = %s\n",
          filename,password,newpassword,debug,query);

    for (index = optind; index < _argc; index++)
    fprintf (stdout,"Non-option argument %s\n", _argv[index]);
  }
  
 
  
  
  if (show_only) return 0;
    
    
  // execute query  
    
    
    sqlite3 *db;
    char *err_msg = 0;
    
    
    int rc = sqlite3_open(filename, &db);
  /*  
    int sqlite3_rekey(
  sqlite3 *db,                    Database to be rekeyed 
  const void *pKey, int nKey     The new key, and the length of the key in bytes 
);
    */
    
    
    if (rc != SQLITE_OK) {
        
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        
        return 1;
    }

    if (password)     sqlite3_key  (db,password,strlen(password));
    if (newpassword)  sqlite3_rekey(db,newpassword,(newpassword=="null") ? 0 : strlen(newpassword));


    if (do_example) {
      char *sql = "DROP TABLE IF EXISTS Cars;" 
                "CREATE TABLE Cars(Id INT, Name TEXT, Price INT);" 
                "INSERT INTO Cars VALUES(67, 'A`udi', 52642);" 
                "INSERT INTO Cars VALUES(2, 'M\"ercedes', 57127);" 
                "INSERT INTO Cars VALUES(3, 'Skoda', 9000);" 
                "INSERT INTO Cars VALUES(4, 'Volvo', 29000);" 
                "INSERT INTO Cars VALUES(500, 'Bentley', 350000);" 
                "INSERT INTO Cars VALUES(6000, 'Citroen', 21000);" 
                "INSERT INTO Cars VALUES(7, 'Hummer', 41400);" 
                "INSERT INTO Cars VALUES(8, 'Volkswagen', null);"
                "UPDATE Cars SET Name='VW' where Name='Volkswagen';"
                ;

      rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
   
    
      if (rc != SQLITE_OK ) {
          
          fprintf(stderr, "SQL error: %s\n", err_msg);
          
          sqlite3_free(err_msg);        
          sqlite3_close(db);
          
          return 1;
      } 
     }
    
  /*     
     char *sql1a="SELECT "
      "json_group_array("
      "	json_object('Id', Id, 'Name', Name,'Price',Price)"
      ") AS json_result"
      "FROM (SELECT * FROM Cars);";
     
     char *sql1a1="SELECT json_group_array(json_object('Id', Id, 'Name', Name,'Price',Price)) AS json_result FROM (SELECT * FROM Cars);";
      

    char *sql12="SELECT json_array(1,2,'3',4)";
       
           char *sql1 = "SELECT * FROM Cars";
           //char *sql1 = "UPDATE Cars Set Price=20000 Where Name='Audi'";

    unsigned char *par=NULL;
    */
  //par=base64_encode(argv[i],strlen(argv[i]),&len);
  //printf("~~~SQL  pass:%s\n~~~SQL query:\n%s\n~~~SQL END\n",_argv[1],_argv[2]);
  //free(par);
  
  
    fprintf(stdout,jr_start);
    sqlite_header=0;    // to force print headers
    rc = sqlite3_exec(db, query, callback, 0, &err_msg);
    fprintf(stdout,jr_end);
    
    if (rc != SQLITE_OK ) {
        
  //      fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL ERROR: %s\n", err_msg);

        sqlite3_free(err_msg);
        sqlite3_close(db);
        
        return 1;
    } 
    fprintf(stdout,"SQL OK\n");   
    int last_id = sqlite3_last_insert_rowid(db);
    fprintf(stdout,"rowid=%d\n", last_id);
    fprintf(stdout,"count=%d\n", sqlite_header);
    fprintf(stdout, "ts(s)=%d\n", time(NULL)-ts);

   
        sqlite3_close(db);
 /*   
    
    	char buffer[1024];
	unsigned int buflen= 1024;
	int err;
    jwOpen( buffer, buflen, JW_OBJECT, JW_PRETTY );  // open root node as object
    jwObj_string( "key", "value" );                  // writes "key":"value"
    jwObj_int( "int", 1 );                           // writes "int":1
    jwObj_array( "anArray");                         // start "anArray": [...] 
        jwArr_int( 0 );                              // add a few integers to the array
        jwArr_int( 1 );
        jwArr_int( 2 );
    jwEnd();                                         // end the array
    err= jwClose();                                  // close root object - done
    
    printf( buffer );
      if( err != JWRITE_OK )
        printf( "Error: %s at function call %d\n", jwErrorToString(err), jwErrorPos() );

	printf("\n\nA JSON array example:\n\n" );
*/
    
    return 0;
}

int callback(void *NotUsed, int argc, char **argv, 
                    char **azColName) {
    
    NotUsed = 0;
    char *sep="|",*nulll="~",*clear="`";
      
    int  check_n(const char *s,int (*foo)(char c,int poz), size_t ile) {
      size_t len=strlen(s);
      len=(ile>0 && ile<len) ?ile:len;
      for (size_t i=0;i<len;i++) if (!foo(s[i],i))   return 0;
      return 1;
    }
    
    int foo_int(char c, int poz) {
        if (poz) return ('0' > c || c > '9') ? 0 : 1;
        return ('1' > c || c > '9') ? 0 : 1; //string started with leading 0
    
    }
    
    int foo_text(char c, int poz) {
         //printf("%i 1%i 2%i 3%i 4%i",c,' ' >= c,c <= '}',c!=sep[0],c!=nulll[0],c!=clear[0]);
         return (' ' <= c && c <= '}' && c!=sep[0] && c!=nulll[0] && c!=clear[0]) ? 1 : 0;
    }
    
    
    if (!sqlite_header++) {
      for (int i = 0; i < argc; i++) 
         fprintf(stdout,"%s%s", i ? sep : "",azColName[i] );   
      fprintf(stdout,"\n");
    
    }
    
    for (int i = 0; i < argc; i++) {
      size_t len=0;
        
        if (i) fprintf(stdout,"%s",sep);
      
        if (!argv[i]) { //null
            fprintf(stdout,nulll);
            continue;
        }
            
        if (check_n(argv[i],foo_int,0)) {
          
            unsigned char *par=base64_long_encode(atol(argv[i]),&len);
      
            if (debug) {
              fprintf(stdout,"=%s (len:%i decode:%i)",par,len,base64_long_decode(par,len) );
              } else {
              fprintf(stdout,"=%s",par);
            }
            
            free(par);
            continue;
        }
        
        if (check_n(argv[i],foo_text,0)) {
            fprintf(stdout,"%s%s",clear,argv[i] );
            continue;
        }
        {
          unsigned char *par=NULL;
          par=base64_encode(argv[i],strlen(argv[i]),&len);
          if (debug) {
            fprintf(stdout,"%s(%s)", par  ,argv[i] );
            } else {
            fprintf(stdout,"%s",par);
          }
          free(par);
        }
    }
    
    fprintf(stdout,"\n");
    
    return 0;
}

