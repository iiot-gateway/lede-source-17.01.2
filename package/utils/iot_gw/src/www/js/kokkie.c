#include <stdio.h>
#include <string.h>

int main() {
    char line[80];
    char zonename[80];
    char zoneabbr[80];
    FILE * fp;
    int i = 0;
    char * s;

    printf( "function tzSelector() {\n" );
    printf( "    var html = \"\";\n" );
    printf( "    html += \"<select id='tz_sel'>\";\n" );
    if ( ( fp = fopen( "kok", "r" ) ) != NULL ) {
        while ( fgets( line, 80, fp ) != NULL ) {
            // printf( "Line: %s", line );
            
            // Remove \n at end
            int len = strlen( line );
            if ( len > 1 ) {
                if ( line[len-1] == '\n' ) line[len-1] = '\0';
                
                if ( ( s = strstr( line, "," ) ) != NULL ) {
                    strcpy( zoneabbr, s+1 );
                    *s = '\0';
                    strcpy( zonename, line );
                }
                
                printf( "    html += \"<option value='%s'>%s</option>\";\n", zoneabbr, zonename );
            }
        }
        fclose( fp );
    }
    printf( "    html += \"</select>\";\n" );
    printf( "    return( html );\n" );
    printf( "}\n" );
    
    return 0;
}
