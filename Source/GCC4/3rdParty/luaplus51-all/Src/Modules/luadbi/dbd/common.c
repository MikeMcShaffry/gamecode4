#include <dbd/common.h>

const char *strlower(char *in) {
    char *s = in;

    while(*s) {
	*s= (*s <= 'Z' && *s >= 'A') ? (*s - 'A') + 'a' : *s;
	s++;
    }

    return in;
}

/*
 * replace '?' placeholders with {native_prefix}\d+ placeholders
 * to be compatible with native API
 */
char *replace_placeholders(lua_State *L, char native_prefix, const char *sql) {
    size_t len = strlen(sql);
    int num_placeholders = 0;
    int extra_space = 0;
    int i;
    char *newsql;
    int newpos = 1;
    int ph_num = 1;
    int in_quote = 0;
    char format_str[4];

    format_str[0] = native_prefix;
    format_str[1] = '%';
    format_str[2] = 'u';
    format_str[3] = '\0';

    /*
     * dumb count of all '?'
     * this will match more placeholders than necessesary
     * but it's safer to allocate more placeholders at the
     * cost of a few bytes than risk a buffer overflow
     */ 
    for (i = 1; i < len; i++) {
	if (sql[i] == '?') {
	    num_placeholders++;
	}
    }
    
    /*
     * this is MAX_PLACEHOLDER_SIZE-1 because the '?' is 
     * replaced with '{native_prefix}'
     */ 
    extra_space = num_placeholders * (MAX_PLACEHOLDER_SIZE-1); 

    /*
     * allocate a new string for the converted SQL statement
     */
    newsql = malloc(sizeof(char) * (len+extra_space+1));
    memset(newsql, 0, sizeof(char) * (len+extra_space+1));
    
    /* 
     * copy first char. In valid SQL this cannot be a placeholder
     */
    newsql[0] = sql[0];

    /* 
     * only replace '?' not in a single quoted string
     */
    for (i = 1; i < len; i++) {
	/*
	 * don't change the quote flag if the ''' is preceded 
	 * by a '\' to account for escaping
	 */
	if (sql[i] == '\'' && sql[i-1] != '\\') {
	    in_quote = !in_quote;
	}

	if (sql[i] == '?' && !in_quote) {
	    size_t n;

	    if (ph_num > MAX_PLACEHOLDERS) {
		luaL_error(L, "Sorry, you are using more than %d placeholders. Use %c{num} format instead", MAX_PLACEHOLDERS, native_prefix);
	    }

	    n = snprintf(&newsql[newpos], MAX_PLACEHOLDER_SIZE, format_str, ph_num++);

	    newpos += n;
	} else {
	    newsql[newpos] = sql[i];
	    newpos++;
	}
    }

    /* 
     * terminate string on the last position 
     */
    newsql[newpos] = '\0';

    /* fprintf(stderr, "[%s]\n", newsql); */
    return newsql;
}

