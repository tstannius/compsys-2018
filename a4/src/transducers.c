#include "transducers.h"
#include <sys/types.h> // Used for getpid(). Not necessary to include this one explicitly.
#include <stdlib.h>
#include <unistd.h> // Used for getpid()

struct stream {
  FILE* f; // File pointer
  int open; // = 1 if file is in use / unavailable. = 0 if file is NOT in use / available.
};

/* file_pipe */
static int file_pipe(FILE* files[2]) {
  int fds[2];
  int r = pipe(fds);
  if (r == 0) {
    files[0] = fdopen(fds[0], "r");
    files[1] = fdopen(fds[1], "w");
    if (files[0] && files[1]) {
      return 0;
    } 
    else {
      return 1;
    }
  } 
  else {
    return r;
  }
}

// Should contain a FILE* pointer and a flag indicating whether the stream already has a reader.
void transducers_free_stream(stream *s) {
  free(s);
}

int transducers_link_source(stream **out,
                            transducers_source s, const void *arg) {
  struct stream * str = malloc(sizeof(stream)); // Create new stream.
  *out = str;
  FILE* files[2];
  int fp = file_pipe(files); // Create pipes from stream

  if (fp != 0) {
    return 1;
  }
  
  pid_t pid; // Fork
  pid = fork();
  
  if (pid == -1) {
    return 1;
  }

  if (pid == 0) /* Child */
  {
    fclose(files[0]);   // Close read-port.
    s(arg,files[1]);    // Write to write-port via source_function
    fclose(files[1]);   // Close write-port.
    exit(0);
  }
  else /* Parent */
  {
    fclose(files[1]);   // Close write-port.
    str->f = files[0];  // Read from read-port.
    str->open = 0;      // Set stream to available.
    return 0;
  }

  return 1;
}

int transducers_link_sink(transducers_sink s, void *arg,
                          stream *in) {
  // Check that stream in is not in use.
  if (in->open == 1) {
    return 1;
  }
  else {
    in->open = 1;
  }
  s(arg,in->f);

  return 0;
}

int transducers_link_1(stream **out,
                       transducers_1 t, const void *arg,
                       stream* in) {
  // Check that stream in is not in use.
  if (in->open == 1) {
    return 1;
  }
  else {
    in->open = 1;
  }
  
  struct stream * str = malloc(sizeof(stream)); // Create new stream.
  *out = str;
  FILE* files[2];
  int fp = file_pipe(files); // Create pipes from stream

  if (fp != 0) {
    return 1;
  }
  
  pid_t pid; // Fork
  pid = fork();
  
  if (pid == -1) {
    return 1;
  }

  if (pid == 0) /* Child */
  {
    fclose(files[0]);   // Close read-port.
    t(arg,files[1],in->f);    // Write to write-port via source_function
    fclose(files[1]);   // Close write-port.
    exit(0);
  }
  else /* Parent */
  {
    fclose(files[1]);   // Close write-port.
    str->f = files[0];  // Read from read-port.
    str->open = 0;      // Set stream to available.
    return 0;
  }

  return 1;
}

int transducers_link_2(stream **out,
                       transducers_2 t, const void *arg,
                       stream* in1, stream* in2) {
  // Check that stream in1 is not in use.
  if (in1->open == 1) {
    return 1;
  }
  else {
    in1->open = 1;
  }
  // Check that stream in2 is not in use. Also ensures in1 != in2.
  if (in2->open == 1) {
    return 1;
  }
  else {
    in2->open = 1;
  }

  FILE* files[2];
  int fp = file_pipe(files); // Create pipes from stream

  if (fp != 0) {
    return 1;
  }
  
  pid_t pid; // Fork
  pid = fork();
  
  if (pid == -1) {
    return 1;
  }

  if (pid == 0) /* Child */
  {
    fclose(files[0]);   // Close read-port.
    t(arg,files[1],in1->f,in2->f);    // Write to write-port via source_function
    fclose(files[1]);   // Close write-port.
    exit(0);
  }

  fclose(files[1]);      
  struct stream * str = malloc(sizeof(stream)); // Create new stream.
  *out = str;
  str->f = files[0];  // Read from read-port.

  return 0;
}


int transducers_dup(stream **out1, stream **out2,
                    stream *in) {
  if (in->open == 1) {
    return 1;
  }
  else {
    in->open = 1;
  }
  struct stream * str1 = malloc(sizeof(stream));
  struct stream * str2 = malloc(sizeof(stream));
  *out1 = str1;
  *out2 = str2;                    
  FILE* files1[2];
  FILE* files2[2];

  int fp1 = file_pipe(files1); // Create pipes from stream
  int fp2 = file_pipe(files2);

  if ( fp2 != 0 || fp1 != 0) {
    return 1;
  }

  pid_t pid; // Fork
  pid = fork();

  if (pid == -1) {
    return 1;
  }

  if (pid == 0) /* Child */
  {
    fclose(files1[0]);   // Close read-port.
    fclose(files2[0]);

    unsigned char d; 
    while (fread(&d, sizeof(unsigned char), 1, (*in).f ) == 1) {
      if (fwrite(&d, sizeof(unsigned char), 1, files1[1] ) != 1 ) {
        break;
      } 
      if (fwrite(&d, sizeof(unsigned char), 1, files2[1] ) != 1 ) {  
        break;
      }       
    }
    
    fclose(files1[1]);   // Close write-port.
    fclose(files2[1]);
    exit(0);
  }
  else /* Parent */
  {
    fclose(files1[1]);   // Close write-port.
    fclose(files2[1]); 
    str1->f = files1[0];  // Read from read-port.
    str2->f = files2[0];  // Read from read-port.
    str1->open = 0;
    str2->open = 0;
  }  
return 0;

}
