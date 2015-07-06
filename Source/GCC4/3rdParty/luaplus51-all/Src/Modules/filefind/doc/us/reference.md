## Reference Manual

This is a reference of all of the filefind module's methods.

## Module `filefind`

**for entry in filefind.match(*pattern*) do** - Begins a new iteration of files and directories using *pattern* as the wildcard.  Simple access to the file system is used.  The more powerful file globbing facilities are not available.

**for entry in filefind.glob(*pattern*) do** - Begins a new iteration of files and/or directories using *pattern* as the glob wildcard.  All glob syntax, described elsewhere, is available.

### File entry information

**entry.filename** - string - The relative path from the *filefind.match()* starting directory.

**entry.creation_time** - number - The creation time of the file/directory in seconds starting from midnight (00:00:00), January 1, 1970.

**entry.access_time** - number - The last access time of the file/directory in seconds starting from midnight (00:00:00), January 1, 1970.

**entry.write_time** - number - The last write time of the file/directory in seconds starting from midnight (00:00:00), January 1, 1970.

**entry.creation_FILETIME** - table - The creation time of the file/directory in FILETIME format starting from midnight (00:00:00), January 1, 1970.  Element 1 of the table is the low file time.  Element 2 is the high file time.

**entry.access_FILETIME** - table - The last access time of the file/directory in FILETIME format starting from midnight (00:00:00), January 1, 1970.  Element 1 of the table is the low file time.  Element 2 is the high file time.

**entry.write_FILETIME** - table - The last write time of the file/directory in FILETIME format starting from midnight (00:00:00), January 1, 1970.  Element 1 of the table is the low file time.  Element 2 is the high file time.

**entry.size** - number - The size of the file.

**entry.is_directory** - boolean - <tt>true</tt> if the glob entry is a directory, <tt>false</tt> otherwise.

**entry.is_readonly** - boolean - <tt>true</tt> if the glob entry is read only, <tt>false</tt> otherwise.

