#include "tsv.h"

static int getBufLength ( FILE* stream ) {
    rewind ( stream );
    int out = 0;
    while ( 1 ) {
        int c = fgetc ( stream );
        if ( c == EOF ) {
            break;
        }
        out++;
    }
    return out;
}

static int getColumnNameLength ( FILE* stream ) {
    rewind ( stream );
    int out = 0;
    while ( 1 ) {
        int c = fgetc ( stream );
        if ( c == EOF ) {
            break;
        }
        if ( c == '\n' ) {
            out++;
            break;
        }
        if ( c == '\t' ) {
            out++;
        }
    }
    return out;
}

static int getDataLength ( FILE* stream ) {
    rewind ( stream );
    TSV_SKIP_LINE ( stream )
    int out = 0;
    while ( 1 ) {
        int c = fgetc ( stream );
        if ( c == EOF ) {
            break;
        }
        if ( c == '\t' || c == '\n' ) {
            out++;
        }
    }
    return out;
}

static void parse ( TSVresult *r, FILE* stream ) {
    rewind ( stream );
    int state = 0;
    int i = 0;
    int j = 0;
    int set = 1;
    while ( 1 ) {
        int c = fgetc ( stream );
        if ( c == EOF ) {
            break;
        }
        if ( i >= r->buf_length ) {
            return;
        }
        if ( c == '\n' || c == '\t' ) {
            r->buf[i] = '\0';
        } else {
            r->buf[i] = c;
        }

        switch ( state ) {
        case 0://header
            if ( c == '\n' ) {
                j = 0;
                set = 1;
                state = 1;
                break;
            }
            if ( c == '\t' ) {
                if ( set ) {
                    if ( j < r->column_name_length ) {
                        j++;
                    }
                    set = 0;
                }
                set = 1;
                break;
            }
            if ( set ) {
                if ( j < r->column_name_length ) {
                    r->column_name[j] = &r->buf[i];
                    j++;
                }
                set = 0;
            }

            break;
        case 1://data row
            if ( c == '\t' || c == '\n' ) {
                if ( set ) {
                    if ( j < r->data_length ) {
                        j++;
                    }
                    set = 0;
                }
                set = 1;
                break;
            }
            if ( set ) {
                if ( j < r->data_length ) {
                    r->data[j] = &r->buf[i];
                    j++;
                }
                set = 0;
            }
            break;
        default:
#ifdef MODE_DEBUG
            fprintf ( stderr, "%s(): unknown state\n", F );
#endif
            break;
        }

        i++;
    }
}

static int TSVcheck ( TSVresult *r ) {
    int row_count = TSVntuples ( r );
    int column_count = r->column_name_length;
    for ( int i = 0; i < row_count; i++ ) {
        for ( int j = 0; j < column_count; j++ ) {
            char * v = TSVgetvalue ( r, i, j );
            if ( v == NULL ) {
#ifdef MODE_DEBUG
                fprintf ( stderr, "%s(): bad value found\n", F );
#endif
                return 0;
            }
        }
    }
    return 1;
}

int TSVinit ( TSVresult *r, const char *path ) {
    FILE* stream = fopen ( path, "r" );
    if ( stream == NULL ) {
#ifdef MODE_DEBUG
        printde ( "%s(): on file: %s - ", F, path );
        perror ( "" );
#endif
        return 0;
    }
    int l = getBufLength ( stream );
    if ( l <= 0 ) {
#ifdef MODE_DEBUG
        fprintf ( stderr, "%s(): %s: not enough data for buffer\n", F,path );
#endif
        fclose ( stream );
        return 0;
    }
    size_t sz = l * sizeof * ( r->buf );
    r->buf = malloc ( sz );
    if ( r->buf == NULL ) {
#ifdef MODE_DEBUG
        fprintf ( stderr, "%s(): %s: failed to allocate memory for buffer: ", F, path );
        perror ( "" );
#endif
        fclose ( stream );
        return 0;
    }
    memset ( r->buf, 0, sz );
    r->buf_length = l;
    l = getColumnNameLength ( stream );
    if ( l <= 0 ) {
#ifdef MODE_DEBUG
        fprintf ( stderr, "%s(): %s: not enough data for column\n", F, path );
#endif
        fclose ( stream );
        return 0;
    }
    sz = l * sizeof * ( r->column_name );
    r->column_name = malloc ( sz );
    if ( r->column_name == NULL ) {
#ifdef MODE_DEBUG
        fprintf ( stderr, "%s(): %s: failed to allocate memory for column_name", F, path );
        perror ( "" );
#endif
        fclose ( stream );
        return 0;
    }
    memset ( r->column_name, 0, sz );
    r->column_name_length = l;
    l = getDataLength ( stream );
    if ( l <= 0 ) {
#ifdef MODE_DEBUG
        fprintf ( stderr, "%s(): %s: not enough data for data\n", F, path );
#endif
        fclose ( stream );
        return 0;
    }
    sz = l * sizeof * ( r->data );
    r->data = malloc ( sz );
    if ( r->data == NULL ) {
#ifdef MODE_DEBUG
        fprintf ( stderr, "%s(): %s: failed to allocate memory for data", F, path );
        perror ( "" );
#endif
        fclose ( stream );
        return 0;
    }
    memset ( r->data, 0, sz );
    r->data_length = l;
    parse ( r, stream );
    fclose ( stream );
    if ( !TSVcheck ( r ) ) {
#ifdef MODE_DEBUG
        fprintf ( stderr, "%s(): %s: checking result is bad\n", F, path );
#endif
        return 0;
    }
    return 1;
}

int TSVntuples ( TSVresult *r ) {
    if ( r->column_name_length == 0 ) {
        return 0;
    }
    return r->data_length / r->column_name_length;
}

char * TSVgetvalue ( TSVresult *r, int row_number, int column_number ) {
    int ind = r->column_name_length * row_number + column_number;
    if ( ind >= r->data_length ) {
        r->null_returned=1;
        return NULL;
    }
    return r->data[ind];
}

char * TSVgetvalues ( TSVresult *r, int row_number, const char * column_name ) {
    for ( int i = 0; i < r->column_name_length; i++ ) {
        if ( strcmp ( column_name, r->column_name[i] ) == 0 ) {
            int ind = r->column_name_length * row_number + i;
            if ( ind >= r->data_length ) {
                printde ( "index out of valid bound for column '%s' and row %d", column_name, row_number );
                r->null_returned=1;
                return NULL;
            }
            return r->data[ind];
        }
    }
    printde ( "column '%s' not found\n", column_name );
    r->null_returned=1;
    return NULL;
}

int TSVgetis ( TSVresult *r, int row_number, const char * column_name ) {
    char *s = TSVgetvalues ( r, row_number, column_name );
    if ( s == NULL ) {
        return 0;
    }
    return atoi ( s );
}

double TSVgetfs ( TSVresult *r, int row_number, const char * column_name ) {
    char *s = TSVgetvalues ( r, row_number, column_name );
    if ( s == NULL ) {
        return 0.0;
    }
    return atof ( s );
}

int TSVgeti ( TSVresult *r, int row_number, int column_number ) {
    char *s = TSVgetvalue ( r, row_number, column_number );
    if ( s == NULL ) {
        return 0;
    }
    return atoi ( s );
}

double TSVgetf ( TSVresult *r, int row_number, int column_number ) {
    char *s = TSVgetvalue ( r, row_number, column_number );
    if ( s == NULL ) {
        return 0.0;
    }
    return atof ( s );
}

int TSVnullreturned ( TSVresult *r ) {
    return r->null_returned;
}
void TSVclear ( TSVresult *r ) {
    free ( r->buf );
    r->buf = NULL;
    r->buf_length = 0;
    free ( r->column_name );
    r->column_name = NULL;
    r->column_name_length = 0;
    free ( r->data );
    r->data = NULL;
    r->data_length = 0;
    r->null_returned=0;
}
