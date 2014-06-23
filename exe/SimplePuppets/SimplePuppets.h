
#ifndef __SIMPLEPUPPETS_h_
#define __SIMPLEPUPPETS_h_

#include <cv.h>

#include <CLM.h>

#include <gtk/gtk.h>

#include <vector>

using namespace std;

GtkWidget *window;
GtkWidget *testwindow;

GtkWidget *button;

GtkWidget *table;

GtkWidget *drawing_area;

GtkWidget *check, *check1, *check2;

GtkWidget *hscale, *hscale2, *hscale3, *hscale4, *hscale5, *label1, *label2, *label3, *label4, *label5, *avatarchoice, *inputchoice;

GtkObject *adj1, *adj2, *adj3, *adj4, *adj5;

// File recording definitions
GtkWidget *record_checkbox;
bool record_global = false;

// A list of avatar files in a '../avatars' directory that will be used to load tghem
vector<pair<string,string> > avatar_files;

vector<pair<string,string> > default_videos;

bool quitmain = 0;
bool GRAYSCALE = false;

GtkWidget *filew, *filez;


#define INFO_STREAM( stream ) \
std::cout << stream << std::endl

#define WARN_STREAM( stream ) \
std::cout << "Warning: " << stream << std::endl

#define ERROR_STREAM( stream ) \
std::cout << "Error: " << stream << std::endl

// TODO most of these need to be removed
IplImage opencvImage;
GdkPixbuf* pix;

bool USEWEBCAM = false;
bool CHANGESOURCE = false;

bool face_replace_global = true;
bool reset_neutral_global = true;
bool write_to_file_global = false;

// This indicates which video file should be read anew
string inputfile = "../videos/changeLighting.wmv";

void use_webcam();
static gboolean time_handler( GtkWidget *widget );
gboolean expose_event_callback(GtkWidget *widget, GdkEventExpose *event, gpointer data);
static void file_ok_sel( GtkWidget *w, GtkFileSelection *fs );
static void file_ok_sel_z( GtkWidget *w, GtkFileSelection *fs );
static void callback( GtkWidget *widget, gpointer data );
static gboolean delete_event( GtkWidget *widget, GdkEvent *event, gpointer data );
static void printErrorAndAbort( const std::string & error );

vector<string> get_arguments(int argc, char **argv);

void doFaceTracking(int argc, char **argv);
void startGTK(int argc, char **argv);
int main (int argc, char **argv);

#endif
