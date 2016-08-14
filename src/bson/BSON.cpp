/* bson.c */

/*    Copyright 2009, 2010 10gen Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <climits>

#include "BSON.h"

const int initialBufferSize = 128;

/* only need one of these */
static const int zero = 0;

/* Custom standard function pointers. */
void *( *bson_malloc_func )( size_t ) = malloc;
void *( *bson_realloc_func )( void *, size_t ) = realloc;
void  ( *bson_free )( void * ) = free;
bson_printf_func bson_printf = printf;
bson_fprintf_func bson_fprintf = fprintf;
bson_sprintf_func bson_sprintf = sprintf;

static int _bson_errprintf( const char *, ... );
bson_printf_func bson_errprintf = _bson_errprintf;

/* ObjectId fuzz functions. */
static int ( *oid_fuzz_func )( void ) = NULL;
static int ( *oid_inc_func )( void )  = NULL;

/* ----------------------------
   READING
   ------------------------------ */

bson *bson_empty( bson *obj ) {
	static char data[] = "\005\0\0\0\0";
	bson_init_data( obj, data );
	obj->finished = 1;
	obj->err = 0;
	obj->stackPos = 0;
	return obj;
}

int bson_copy( bson *out, const bson *in ) {
	if ( !out ) return BSON_ERROR;
	if ( !in->finished ) return BSON_ERROR;
	bson_init_size( out, bson_size( in ) );
	memcpy( out->data, in->data, bson_size( in ) );
	out->finished = 1;

	return BSON_OK;
}

int bson_init_data( bson *b, char *data ) {
	b->data = data;
	b->dataSize = INT_MAX; // no overflow detection for bson_iterator_next
	return BSON_OK;
}

int bson_init_data_size( bson *b, char *data, int size ) {
	b->data = data;
	b->dataSize = size; // used for overflow detection for bson_iterator_next
	return BSON_OK;
}

int bson_init_finished_data( bson *b, char *data ) {
	bson_init_data( b, data );
	b->stackPos = 0;
	b->finished = 1;
	return BSON_OK;
}

static void _bson_reset( bson *b ) {
	b->finished = 0;
	b->stackPos = 0;
	b->err = 0;
	b->errstr = NULL;
}

int bson_size( const bson *b ) {
	int i;
	if ( ! b || ! b->data )
		return 0;
	bson_little_endian32( &i, b->data );
	return i;
}

const char *bson_data( bson *b ) {
	return (const char *)b->data;
}

static char hexbyte( char hex ) {
	switch ( hex ) {
	case '0':
		return 0x0;
	case '1':
		return 0x1;
	case '2':
		return 0x2;
	case '3':
		return 0x3;
	case '4':
		return 0x4;
	case '5':
		return 0x5;
	case '6':
		return 0x6;
	case '7':
		return 0x7;
	case '8':
		return 0x8;
	case '9':
		return 0x9;
	case 'a':
	case 'A':
		return 0xa;
	case 'b':
	case 'B':
		return 0xb;
	case 'c':
	case 'C':
		return 0xc;
	case 'd':
	case 'D':
		return 0xd;
	case 'e':
	case 'E':
		return 0xe;
	case 'f':
	case 'F':
		return 0xf;
	default:
		return 0x0; /* something smarter? */
	}
}

void bson_oid_from_string( bson_oid_t *oid, const char *str ) {
	int i;
	for ( i=0; i<12; i++ ) {
		oid->bytes[i] = ( hexbyte( str[2*i] ) << 4 ) | hexbyte( str[2*i + 1] );
	}
}

void bson_oid_to_string( const bson_oid_t *oid, char *str ) {
	static const char hex[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
	int i;
	for ( i=0; i<12; i++ ) {
		str[2*i]	 = hex[( oid->bytes[i] & 0xf0 ) >> 4];
		str[2*i + 1] = hex[ oid->bytes[i] & 0x0f	  ];
	}
	str[24] = '\0';
}

void bson_set_oid_fuzz( int ( *func )( void ) ) {
	oid_fuzz_func = func;
}

void bson_set_oid_inc( int ( *func )( void ) ) {
	oid_inc_func = func;
}

void bson_oid_gen( bson_oid_t *oid ) {
	static int incr = 0;
	static int fuzz = 0;
	int i;
	int t = time( NULL );

	if( oid_inc_func )
		i = oid_inc_func();
	else
		i = incr++;

	if ( !fuzz ) {
		if ( oid_fuzz_func )
			fuzz = oid_fuzz_func();
		else {
			srand( t );
			fuzz = rand();
		}
	}

	bson_big_endian32( &oid->ints[0], &t );
	oid->ints[1] = fuzz;
	bson_big_endian32( &oid->ints[2], &i );
}

time_t bson_oid_generated_time( bson_oid_t *oid ) {
	time_t out;
	bson_big_endian32( &out, &oid->ints[0] );

	return out;
}

void bson_print( bson *b ) {
	bson_print_raw( b->data , 0 );
}

void bson_print_raw( const char *data , int depth ) {
	bson_iterator i;
	const char *key;
	int temp;
	bson_timestamp_t ts;
	char oidhex[25];
	bson scope;
	bson_iterator_from_buffer( &i, data );

	while ( bson_iterator_next( &i ) ) {
		bson_type t = bson_iterator_type( &i );
		if ( t == 0 )
			break;
		key = bson_iterator_key( &i );

		for ( temp=0; temp<=depth; temp++ )
			bson_printf( "\t" );
		bson_printf( "%s : %d \t " , key , t );
		switch ( t ) {
		case BSON_DOUBLE:
			bson_printf( "%f" , bson_iterator_double( &i ) );
			break;
		case BSON_STRING:
			bson_printf( "%s" , bson_iterator_string( &i ) );
			break;
		case BSON_SYMBOL:
			bson_printf( "SYMBOL: %s" , bson_iterator_string( &i ) );
			break;
		case BSON_OID:
			bson_oid_to_string( bson_iterator_oid( &i ), oidhex );
			bson_printf( "%s" , oidhex );
			break;
		case BSON_BOOL:
			bson_printf( "%s" , bson_iterator_bool( &i ) ? "true" : "false" );
			break;
		case BSON_DATE:
			bson_printf( "%ld" , ( long int )bson_iterator_date( &i ) );
			break;
		case BSON_BINDATA:
			bson_printf( "BSON_BINDATA" );
			break;
		case BSON_UNDEFINED:
			bson_printf( "BSON_UNDEFINED" );
			break;
		case BSON_NULL:
			bson_printf( "BSON_NULL" );
			break;
		case BSON_REGEX:
			bson_printf( "BSON_REGEX: %s", bson_iterator_regex( &i ) );
			break;
		case BSON_CODE:
			bson_printf( "BSON_CODE: %s", bson_iterator_code( &i ) );
			break;
		case BSON_CODEWSCOPE:
			bson_printf( "BSON_CODE_W_SCOPE: %s", bson_iterator_code( &i ) );
			bson_init( &scope );
			bson_iterator_code_scope( &i, &scope );
			bson_printf( "\n\t SCOPE: " );
			bson_print( &scope );
			break;
		case BSON_INT:
			bson_printf( "%d" , bson_iterator_int( &i ) );
			break;
		case BSON_LONG:
			bson_printf( "%lld" , ( uint64_t )bson_iterator_long( &i ) );
			break;
		case BSON_TIMESTAMP:
			ts = bson_iterator_timestamp( &i );
			bson_printf( "i: %d, t: %d", ts.i, ts.t );
			break;
		case BSON_OBJECT:
		case BSON_ARRAY:
			bson_printf( "\n" );
			bson_print_raw( bson_iterator_value( &i ) , depth + 1 );
			break;
		default:
			bson_errprintf( "can't print type : %d\n" , t );
		}
		bson_printf( "\n" );
	}
}

/* ----------------------------
   ITERATOR
   ------------------------------ */

void bson_iterator_init( bson_iterator *i, const bson *b ) {
	i->cur = b->data + 4;
	i->first = 1;
	i->last = b->data + b->dataSize;
}

void bson_iterator_from_buffer( bson_iterator *i, const char *buffer ) {
	i->cur = buffer + 4;
	i->first = 1;
	i->last = NULL;
}

bson_type bson_find( bson_iterator *it, const bson *obj, const char *name ) {
	bson_iterator_init( it, (bson *)obj );
	while( bson_iterator_next( it ) ) {
		if ( strcmp( name, bson_iterator_key( it ) ) == 0 )
			break;
	}
	return bson_iterator_type( it );
}

bson_bool_t bson_iterator_more( const bson_iterator *i ) {
	if (i->last && i->cur >= i->last)
		return BSON_EOO;
	return *( i->cur );
}

bson_type bson_iterator_next( bson_iterator *i ) {
	int ds;

	if ( i->first ) {
		i->first = 0;
		return ( bson_type )( *i->cur );
	}

	switch ( bson_iterator_type( i ) ) {
	case BSON_EOO:
		return BSON_EOO; /* don't advance */
	case BSON_UNDEFINED:
	case BSON_NULL:
		ds = 0;
		break;
	case BSON_BOOL:
		ds = 1;
		break;
	case BSON_INT:
		ds = 4;
		break;
	case BSON_LONG:
	case BSON_DOUBLE:
	case BSON_TIMESTAMP:
	case BSON_DATE:
		ds = 8;
		break;
	case BSON_OID:
		ds = 12;
		break;
	case BSON_STRING:
	case BSON_SYMBOL:
	case BSON_CODE:
		ds = 4 + bson_iterator_int_raw( i );
		break;
	case BSON_BINDATA:
		ds = 5 + bson_iterator_int_raw( i );
		break;
	case BSON_OBJECT:
	case BSON_ARRAY:
	case BSON_CODEWSCOPE:
		ds = bson_iterator_int_raw( i );
		break;
	case BSON_DBREF:
		ds = 4+12 + bson_iterator_int_raw( i );
		break;
	case BSON_REGEX: {
		const char *s = bson_iterator_value( i );
		const char *p = s;
		p += strlen( p )+1;
		p += strlen( p )+1;
		ds = p-s;
		break;
	}

	default: {
		char msg[] = "unknown type: 000000000000";
		bson_numstr( msg+14, ( unsigned )( i->cur[0] ) );
		bson_fatal_msg( 0, msg );
		return (bson_type)0;
	}
	}

	i->cur += 1 + strlen( i->cur + 1 ) + 1 + ds;

	if (i->last && i->cur >= i->last)
		return BSON_EOO;
	return ( bson_type )( *i->cur );
}

bson_type bson_iterator_type( const bson_iterator *i ) {
	return ( bson_type )i->cur[0];
}

const char *bson_iterator_key( const bson_iterator *i ) {
	return i->cur + 1;
}

const char *bson_iterator_value( const bson_iterator *i ) {
	const char *t = i->cur + 1;
	t += strlen( t ) + 1;
	return t;
}

/* types */

int bson_iterator_int_raw( const bson_iterator *i ) {
	int out;
	bson_little_endian32( &out, bson_iterator_value( i ) );
	return out;
}

double bson_iterator_double_raw( const bson_iterator *i ) {
	double out;
	bson_little_endian64( &out, bson_iterator_value( i ) );
	return out;
}

int64_t bson_iterator_long_raw( const bson_iterator *i ) {
	int64_t out;
	bson_little_endian64( &out, bson_iterator_value( i ) );
	return out;
}

bson_bool_t bson_iterator_bool_raw( const bson_iterator *i ) {
	return bson_iterator_value( i )[0];
}

bson_oid_t *bson_iterator_oid( const bson_iterator *i ) {
	return ( bson_oid_t * )bson_iterator_value( i );
}

int bson_iterator_int( const bson_iterator *i ) {
	switch ( bson_iterator_type( i ) ) {
	case BSON_INT:
		return bson_iterator_int_raw( i );
	case BSON_LONG:
		return bson_iterator_long_raw( i );
	case BSON_DOUBLE:
		return bson_iterator_double_raw( i );
	default:
		return 0;
	}
}

double bson_iterator_double( const bson_iterator *i ) {
	switch ( bson_iterator_type( i ) ) {
	case BSON_INT:
		return bson_iterator_int_raw( i );
	case BSON_LONG:
		return bson_iterator_long_raw( i );
	case BSON_DOUBLE:
		return bson_iterator_double_raw( i );
	default:
		return 0;
	}
}

int64_t bson_iterator_long( const bson_iterator *i ) {
	switch ( bson_iterator_type( i ) ) {
	case BSON_INT:
		return bson_iterator_int_raw( i );
	case BSON_LONG:
		return bson_iterator_long_raw( i );
	case BSON_DOUBLE:
		return bson_iterator_double_raw( i );
	default:
		return 0;
	}
}

bson_timestamp_t bson_iterator_timestamp( const bson_iterator *i ) {
	bson_timestamp_t ts;
	bson_little_endian32( &( ts.i ), bson_iterator_value( i ) );
	bson_little_endian32( &( ts.t ), bson_iterator_value( i ) + 4 );
	return ts;
}

bson_bool_t bson_iterator_bool( const bson_iterator *i ) {
	switch ( bson_iterator_type( i ) ) {
	case BSON_BOOL:
		return bson_iterator_bool_raw( i );
	case BSON_INT:
		return bson_iterator_int_raw( i ) != 0;
	case BSON_LONG:
		return bson_iterator_long_raw( i ) != 0;
	case BSON_DOUBLE:
		return bson_iterator_double_raw( i ) != 0;
	case BSON_EOO:
	case BSON_NULL:
		return 0;
	default:
		return 1;
	}
}

const char *bson_iterator_string( const bson_iterator *i ) {
	return bson_iterator_value( i ) + 4;
}

int bson_iterator_string_len( const bson_iterator *i ) {
	return bson_iterator_int_raw( i );
}

const char *bson_iterator_code( const bson_iterator *i ) {
	switch ( bson_iterator_type( i ) ) {
	case BSON_STRING:
	case BSON_CODE:
		return bson_iterator_value( i ) + 4;
	case BSON_CODEWSCOPE:
		return bson_iterator_value( i ) + 8;
	default:
		return NULL;
	}
}

void bson_iterator_code_scope( const bson_iterator *i, bson *scope ) {
	if ( bson_iterator_type( i ) == BSON_CODEWSCOPE ) {
		int code_len;
		bson_little_endian32( &code_len, bson_iterator_value( i )+4 );
		bson_init_data( scope, (char*)((void *)(bson_iterator_value(i)+8+code_len )));
		_bson_reset( scope );
		scope->finished = 1;
	} else {
		bson_empty( scope );
	}
}

bson_date_t bson_iterator_date( const bson_iterator *i ) {
	return bson_iterator_long_raw( i );
}

time_t bson_iterator_time_t( const bson_iterator *i ) {
	return bson_iterator_date( i ) / 1000;
}

int bson_iterator_bin_len( const bson_iterator *i ) {
	return ( bson_iterator_bin_type( i ) == BSON_BIN_BINARY_OLD )
		   ? bson_iterator_int_raw( i ) - 4
		   : bson_iterator_int_raw( i );
}

char bson_iterator_bin_type( const bson_iterator *i ) {
	return bson_iterator_value( i )[4];
}

const char *bson_iterator_bin_data( const bson_iterator *i ) {
	return ( bson_iterator_bin_type( i ) == BSON_BIN_BINARY_OLD )
		   ? bson_iterator_value( i ) + 9
		   : bson_iterator_value( i ) + 5;
}

const char *bson_iterator_regex( const bson_iterator *i ) {
	return bson_iterator_value( i );
}

const char *bson_iterator_regex_opts( const bson_iterator *i ) {
	const char *p = bson_iterator_value( i );
	return p + strlen( p ) + 1;

}

void bson_iterator_subobject( const bson_iterator *i, bson *sub ) {
	bson_init_data( sub, ( char * )bson_iterator_value( i ) );
	_bson_reset( sub );
	sub->finished = 1;
}

void bson_iterator_subiterator( const bson_iterator *i, bson_iterator *sub ) {
	bson_iterator_from_buffer( sub, bson_iterator_value( i ) );
}

/* ----------------------------
   BUILDING
   ------------------------------ */

static void _bson_init_size( bson *b, int size ) {
	if( size == 0 )
		b->data = NULL;
	else
		b->data = ( char * )bson_malloc( size );
	b->dataSize = size;
	b->cur = b->data + 4;
	_bson_reset( b );
}

void bson_init( bson *b ) {
	_bson_init_size( b, initialBufferSize );
}

void bson_init_size( bson *b, int size ) {
	_bson_init_size( b, size );
}

static void bson_append_byte( bson *b, char c ) {
	b->cur[0] = c;
	b->cur++;
}

static void bson_append( bson *b, const void *data, int len ) {
	memcpy( b->cur , data , len );
	b->cur += len;
}

static void bson_append32( bson *b, const void *data ) {
	bson_little_endian32( b->cur, data );
	b->cur += 4;
}

static void bson_append64( bson *b, const void *data ) {
	bson_little_endian64( b->cur, data );
	b->cur += 8;
}

int bson_ensure_space( bson *b, const int bytesNeeded ) {
	int pos = b->cur - b->data;
	char *orig = b->data;
	int new_size;

	if ( pos + bytesNeeded <= b->dataSize )
		return BSON_OK;

	new_size = 1.5 * ( b->dataSize + bytesNeeded );

	if( new_size < b->dataSize ) {
		if( ( b->dataSize + bytesNeeded ) < INT_MAX )
			new_size = INT_MAX;
		else {
			b->err = BSON_SIZE_OVERFLOW;
			return BSON_ERROR;
		}
	}

	b->data = (char*)bson_realloc( b->data, new_size );
	if ( !b->data )
		bson_fatal_msg( !!b->data, "realloc() failed" );

	b->dataSize = new_size;
	b->cur += b->data - orig;

	return BSON_OK;
}

int bson_finish( bson *b ) {
	int i;

	if( b->err & BSON_NOT_UTF8 )
		return BSON_ERROR;

	if ( ! b->finished ) {
		if ( bson_ensure_space( b, 1 ) == BSON_ERROR ) return BSON_ERROR;
		bson_append_byte( b, 0 );
		i = b->cur - b->data;
		bson_little_endian32( b->data, &i );
		b->finished = 1;
	}

	return BSON_OK;
}

void bson_destroy( bson *b ) {
	bson_free( b->data );
	b->err = 0;
	b->data = 0;
	b->cur = 0;
	b->finished = 1;
}

static int bson_append_estart( bson *b, int type, const char *name, const int dataSize ) {
	const int len = strlen( name ) + 1;

	if ( b->finished ) {
		b->err |= BSON_ALREADY_FINISHED;
		return BSON_ERROR;
	}

	if ( bson_ensure_space( b, 1 + len + dataSize ) == BSON_ERROR ) {
		return BSON_ERROR;
	}

	if( bson_check_field_name( b, ( const char * )name, len - 1 ) == BSON_ERROR ) {
		bson_builder_error( b );
		return BSON_ERROR;
	}

	bson_append_byte( b, ( char )type );
	bson_append( b, name, len );
	return BSON_OK;
}

/* ----------------------------
   BUILDING TYPES
   ------------------------------ */

int bson_append_int( bson *b, const char *name, const int i ) {
	if ( bson_append_estart( b, BSON_INT, name, 4 ) == BSON_ERROR )
		return BSON_ERROR;
	bson_append32( b , &i );
	return BSON_OK;
}

int bson_append_long( bson *b, const char *name, const int64_t i ) {
	if ( bson_append_estart( b , BSON_LONG, name, 8 ) == BSON_ERROR )
		return BSON_ERROR;
	bson_append64( b , &i );
	return BSON_OK;
}

int bson_append_double( bson *b, const char *name, const double d ) {
	if ( bson_append_estart( b, BSON_DOUBLE, name, 8 ) == BSON_ERROR )
		return BSON_ERROR;
	bson_append64( b , &d );
	return BSON_OK;
}

int bson_append_bool( bson *b, const char *name, const bson_bool_t i ) {
	if ( bson_append_estart( b, BSON_BOOL, name, 1 ) == BSON_ERROR )
		return BSON_ERROR;
	bson_append_byte( b , i != 0 );
	return BSON_OK;
}

int bson_append_null( bson *b, const char *name ) {
	if ( bson_append_estart( b , BSON_NULL, name, 0 ) == BSON_ERROR )
		return BSON_ERROR;
	return BSON_OK;
}

int bson_append_undefined( bson *b, const char *name ) {
	if ( bson_append_estart( b, BSON_UNDEFINED, name, 0 ) == BSON_ERROR )
		return BSON_ERROR;
	return BSON_OK;
}

static int bson_append_string_base( bson *b, const char *name,
							 const char *value, int len, bson_type type ) {

	int sl = len + 1;
	if ( bson_check_string( b, ( const char * )value, sl - 1 ) == BSON_ERROR )
		return BSON_ERROR;
	if ( bson_append_estart( b, type, name, 4 + sl ) == BSON_ERROR ) {
		return BSON_ERROR;
	}
	bson_append32( b , &sl );
	bson_append( b , value , sl - 1 );
	bson_append( b , "\0" , 1 );
	return BSON_OK;
}

int bson_append_string( bson *b, const char *name, const char *value ) {
	return bson_append_string_base( b, name, value, strlen ( value ), BSON_STRING );
}

int bson_append_symbol( bson *b, const char *name, const char *value ) {
	return bson_append_string_base( b, name, value, strlen ( value ), BSON_SYMBOL );
}

int bson_append_code( bson *b, const char *name, const char *value ) {
	return bson_append_string_base( b, name, value, strlen ( value ), BSON_CODE );
}

int bson_append_string_n( bson *b, const char *name, const char *value, int len ) {
	return bson_append_string_base( b, name, value, len, BSON_STRING );
}

int bson_append_symbol_n( bson *b, const char *name, const char *value, int len ) {
	return bson_append_string_base( b, name, value, len, BSON_SYMBOL );
}

int bson_append_code_n( bson *b, const char *name, const char *value, int len ) {
	return bson_append_string_base( b, name, value, len, BSON_CODE );
}

int bson_append_code_w_scope_n( bson *b, const char *name,
								const char *code, int len, const bson *scope ) {

	int sl = len + 1;
	int size = 4 + 4 + sl + bson_size( scope );
	if ( bson_append_estart( b, BSON_CODEWSCOPE, name, size ) == BSON_ERROR )
		return BSON_ERROR;
	bson_append32( b, &size );
	bson_append32( b, &sl );
	bson_append( b, code, sl );
	bson_append( b, scope->data, bson_size( scope ) );
	return BSON_OK;
}

int bson_append_code_w_scope( bson *b, const char *name, const char *code, const bson *scope ) {
	return bson_append_code_w_scope_n( b, name, code, strlen ( code ), scope );
}

int bson_append_binary( bson *b, const char *name, char type, const char *str, int len ) {
	if ( type == BSON_BIN_BINARY_OLD ) {
		int subtwolen = len + 4;
		if ( bson_append_estart( b, BSON_BINDATA, name, 4+1+4+len ) == BSON_ERROR )
			return BSON_ERROR;
		bson_append32( b, &subtwolen );
		bson_append_byte( b, type );
		bson_append32( b, &len );
		bson_append( b, str, len );
	} else {
		if ( bson_append_estart( b, BSON_BINDATA, name, 4+1+len ) == BSON_ERROR )
			return BSON_ERROR;
		bson_append32( b, &len );
		bson_append_byte( b, type );
		bson_append( b, str, len );
	}
	return BSON_OK;
}

int bson_append_oid( bson *b, const char *name, const bson_oid_t *oid ) {
	if ( bson_append_estart( b, BSON_OID, name, 12 ) == BSON_ERROR )
		return BSON_ERROR;
	bson_append( b , oid , 12 );
	return BSON_OK;
}

int bson_append_new_oid( bson *b, const char *name ) {
	bson_oid_t oid;
	bson_oid_gen( &oid );
	return bson_append_oid( b, name, &oid );
}

int bson_append_regex( bson *b, const char *name, const char *pattern, const char *opts ) {
	const int plen = strlen( pattern )+1;
	const int olen = strlen( opts )+1;
	if ( bson_append_estart( b, BSON_REGEX, name, plen + olen ) == BSON_ERROR )
		return BSON_ERROR;
	if ( bson_check_string( b, pattern, plen - 1 ) == BSON_ERROR )
		return BSON_ERROR;
	bson_append( b , pattern , plen );
	bson_append( b , opts , olen );
	return BSON_OK;
}

int bson_append_bson( bson *b, const char *name, const bson *bson ) {
	if ( bson_append_estart( b, BSON_OBJECT, name, bson_size( bson ) ) == BSON_ERROR )
		return BSON_ERROR;
	bson_append( b , bson->data , bson_size( bson ) );
	return BSON_OK;
}

int bson_append_element( bson *b, const char *name_or_null, const bson_iterator *elem ) {
	bson_iterator next = *elem;
	int size;

	bson_iterator_next( &next );
	size = next.cur - elem->cur;

	if ( name_or_null == NULL ) {
		if( bson_ensure_space( b, size ) == BSON_ERROR )
			return BSON_ERROR;
		bson_append( b, elem->cur, size );
	} else {
		int data_size = size - 2 - strlen( bson_iterator_key( elem ) );
		bson_append_estart( b, elem->cur[0], name_or_null, data_size );
		bson_append( b, bson_iterator_value( elem ), data_size );
	}

	return BSON_OK;
}

int bson_append_timestamp( bson *b, const char *name, bson_timestamp_t *ts ) {
	if ( bson_append_estart( b, BSON_TIMESTAMP, name, 8 ) == BSON_ERROR ) return BSON_ERROR;

	bson_append32( b , &( ts->i ) );
	bson_append32( b , &( ts->t ) );

	return BSON_OK;
}

int bson_append_date( bson *b, const char *name, bson_date_t millis ) {
	if ( bson_append_estart( b, BSON_DATE, name, 8 ) == BSON_ERROR ) return BSON_ERROR;
	bson_append64( b , &millis );
	return BSON_OK;
}

int bson_append_time_t( bson *b, const char *name, time_t secs ) {
	return bson_append_date( b, name, ( bson_date_t )secs * 1000 );
}

int bson_append_start_object( bson *b, const char *name ) {
	if ( bson_append_estart( b, BSON_OBJECT, name, 5 ) == BSON_ERROR ) return BSON_ERROR;
	b->stack[ b->stackPos++ ] = b->cur - b->data;
	bson_append32( b , &zero );
	return BSON_OK;
}

int bson_append_start_array( bson *b, const char *name ) {
	if ( bson_append_estart( b, BSON_ARRAY, name, 5 ) == BSON_ERROR ) return BSON_ERROR;
	b->stack[ b->stackPos++ ] = b->cur - b->data;
	bson_append32( b , &zero );
	return BSON_OK;
}

int bson_append_finish_object( bson *b ) {
	char *start;
	int i;
	if ( bson_ensure_space( b, 1 ) == BSON_ERROR ) return BSON_ERROR;
	bson_append_byte( b , 0 );

	start = b->data + b->stack[ --b->stackPos ];
	i = b->cur - start;
	bson_little_endian32( start, &i );

	return BSON_OK;
}

int bson_append_finish_array( bson *b ) {
	return bson_append_finish_object( b );
}


/* Error handling and allocators. */

static bson_err_handler err_handler = NULL;

bson_err_handler set_bson_err_handler( bson_err_handler func ) {
	bson_err_handler old = err_handler;
	err_handler = func;
	return old;
}

void *bson_malloc( int size ) {
	void *p;
	p = bson_malloc_func( size );
	bson_fatal_msg( !!p, "malloc() failed" );
	return p;
}

void *bson_realloc( void *ptr, int size ) {
	void *p;
	p = bson_realloc_func( ptr, size );
	bson_fatal_msg( !!p, "realloc() failed" );
	return p;
}

int _bson_errprintf( const char *format, ... ) {
	va_list ap;
	int ret;
	va_start( ap, format );
	ret = vfprintf( stderr, format, ap );
	va_end( ap );

	return ret;
}

/**
 * This method is invoked when a non-fatal bson error is encountered.
 * Calls the error handler if available.
 *
 *  @param
 */
void bson_builder_error( bson *b ) {
	if( err_handler )
		err_handler( "BSON error." );
}

void bson_fatal( int ok ) {
	bson_fatal_msg( ok, "" );
}

void bson_fatal_msg( int ok , const char *msg ) {
	if ( ok )
		return;

	if ( err_handler ) {
		err_handler( msg );
	}

	bson_errprintf( "error: %s\n" , msg );
	exit( -5 );
}


/* Efficiently copy an integer to a string. */

void bson_numstr( char *str, int i ) {
	if( i < 1000 )
		memcpy( str, bson_numstrs[i], 4 );
	else
		bson_sprintf( str,"%d", i );
}

/* encoding.c */

/*
 * Copyright 2009-2011 10gen, Inc.
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
 */

/*
 * Portions Copyright 2001 Unicode, Inc.
 *
 * Disclaimer
 *
 * This source code is provided as is by Unicode, Inc. No claims are
 * made as to fitness for any particular purpose. No warranties of any
 * kind are expressed or implied. The recipient agrees to determine
 * applicability of information provided. If this file has been
 * purchased on magnetic or optical media from Unicode, Inc., the
 * sole remedy for any claim will be exchange of defective media
 * within 90 days of receipt.
 *
 * Limitations on Rights to Redistribute This Code
 *
 * Unicode, Inc. hereby grants the right to freely use the information
 * supplied in this file in the creation of products supporting the
 * Unicode Standard, and to make copies of this file in any form
 * for internal or external distribution as long as this notice
 * remains attached.
 */

/*
 * Index into the table below with the first byte of a UTF-8 sequence to
 * get the number of trailing bytes that are supposed to follow it.
 */
static const char trailingBytesForUTF8[256] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

/* --------------------------------------------------------------------- */

/*
 * Utility routine to tell whether a sequence of bytes is legal UTF-8.
 * This must be called with the length pre-determined by the first byte.
 * The length can be set by:
 *  length = trailingBytesForUTF8[*source]+1;
 * and the sequence is illegal right away if there aren't that many bytes
 * available.
 * If presented with a length > 4, this returns 0.  The Unicode
 * definition of UTF-8 goes up to 4-byte sequences.
 */
static int isLegalUTF8( const unsigned char *source, int length ) {
	unsigned char a;
	const unsigned char *srcptr = source + length;
	switch ( length ) {
	default:
		return 0;
		/* Everything else falls through when "true"... */
	case 4:
		if ( ( a = ( *--srcptr ) ) < 0x80 || a > 0xBF ) return 0;
	case 3:
		if ( ( a = ( *--srcptr ) ) < 0x80 || a > 0xBF ) return 0;
	case 2:
		if ( ( a = ( *--srcptr ) ) > 0xBF ) return 0;
		switch ( *source ) {
			/* no fall-through in this inner switch */
		case 0xE0:
			if ( a < 0xA0 ) return 0;
			break;
		case 0xF0:
			if ( a < 0x90 ) return 0;
			break;
		case 0xF4:
			if ( a > 0x8F ) return 0;
			break;
		default:
			if ( a < 0x80 ) return 0;
		}
	case 1:
		if ( *source >= 0x80 && *source < 0xC2 ) return 0;
		if ( *source > 0xF4 ) return 0;
	}
	return 1;
}

static int bson_validate_string( bson *b, const unsigned char *string,
                                 const int length, const char check_utf8, const char check_dot,
                                 const char check_dollar ) {

	int position = 0;
	int sequence_length = 1;

	if( check_dollar && string[0] == '$' ) {
		b->err |= BSON_FIELD_INIT_DOLLAR;
	}

	while ( position < length ) {
		if ( check_dot && *( string + position ) == '.' ) {
			b->err |= BSON_FIELD_HAS_DOT;
		}

		if ( check_utf8 ) {
			sequence_length = trailingBytesForUTF8[*( string + position )] + 1;
			if ( ( position + sequence_length ) > length ) {
				b->err |= BSON_NOT_UTF8;
				return BSON_ERROR;
			}
			if ( !isLegalUTF8( string + position, sequence_length ) ) {
				b->err |= BSON_NOT_UTF8;
				return BSON_ERROR;
			}
		}
		position += sequence_length;
	}

	return BSON_OK;
}


int bson_check_string( bson *b, const char *string,
                       const int length ) {

	return bson_validate_string( b, ( const unsigned char * )string, length, 1, 0, 0 );
}

int bson_check_field_name( bson *b, const char *string,
                           const int length ) {

	return bson_validate_string( b, ( const unsigned char * )string, length, 1, 1, 1 );
}
