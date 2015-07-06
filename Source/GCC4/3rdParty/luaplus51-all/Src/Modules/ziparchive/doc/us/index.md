## Introduction

The module *ziparchive* allows high performance read and write access to .zip files.



## Example

<pre>
    require 'ziparchive'

    local archive = ziparchive.open('myfile.zip')
    for entry in archive.files() do
    	print(entry)
    end
</pre>



-------------------------
## Compression Methods and Levels

A number of `ziparchive` functions take parameters for `compressionMethod` and `compressionLevel`.

The parameter named `compressionMethod` may be either `ziparchive.DEFLATED` (the default if not specified) or `ziparchive.UNCOMPRESSED`.  `ziparchive.DEFLATED` is a zlib compression term indicating the file should be compressed according to the specified `compressionLevel`.  `ziparchive.UNCOMPRESSED` will simply store the uncompressed contents of the file in the zip archive.

`compressionLevel` indicates how tightly compressed a `ziparchive.DEFLATED` file should be.  It is a value from `0` to `9` or a helper enumeration `ziparchive.NO_COMPRESSION` (0), `ziparchive.BEST_SPEED` (1), `ziparchive.BEST_COMPRESSION` (9), or `ziparchive.DEFAULT_COMPRESSION` (6).

* A compression level of `0` means the file will not be compressed, but this does not mean the same thing as setting the `compressionMethod` to `ziparchive.UNCOMPRESSED` and will usually result in the 'compressed' file size being slightly larger than the uncompressed file size.
* A compression level of `1` will result in the best compression speed but the worst compressed size (next to an uncompressed file).
* A compression level of `9` will result in the best compressed size, but the time to compress will be longer.




-------------------------
## ziparchive Reference

### ziparchive.help()

Returns a string with a short summary of the ziparchive API.



### archive = ziparchive.new()

Returns an archive object.



### archive = ziparchive.open(filename [, mode = 'r'] [, openflags = 0] [, defaultPassword])

Creates or opens a zip archive.  Returns a ziparchive object if opened successfully.

* `filename` is the name of the zip archive to create/open.
* `mode` is an optional parameter that defaults to 'r', meaning to open an existing archive read-only.  If `'w'` is specified, a new archive is created.  If `'a'` is specified, an existing archive is opened writable (in an 'append' state) or a new archive is created.
* `openflags` is an optional parameter.
** If `ziparchive.SUPPORT_MD5` is specified, md5sums are calculated for every file.
** If `ziparchive.EXTRA_DIRECTORY_AT_BEGINNING` is specified, an extra directory structure is written at the beginning of the file.  This is useful when using zip archives on DVD and avoiding an extra seek.
* `defaultPassword` is an optional password used to encrypt the zip file.


### time = ziparchive.AdjustTime_t(timeToAdjust)

Adjusts a timestamp to zip format.



-------------------------
## archive Reference

### opened = archive:open(fileName [, mode = 'r'] [, openflags = 0] [, defaultPassword])

See the documentation for `ziparchive.open` above.

Returns `true` if the archive was opened successfully, `false` otherwise.



### archive:close()

Closes an open archive.




### archive:updatemd5s()

Updates the md5sums for all files in the archive.



### archive:flush()

While being edited, the archive's directory is not written to disk.  Calling `archive:flush()` will write the archive's directory to disk.



### vfile = archive:filecreate(filename [, compressionMethod = ziparchive.DEFLATED] [, compressionLevel = ziparchive.DEFAULT_COMPRESSION] [, time_t fileTime])

Creates a file entry within the archive.  Returns a userdata representing the file in the archive.

* `filename` is the name of the file to create within the archive.
* `compressionMethod`: See above.
* `compressionLevel`: See above.
* `fileTime` is the time in time_t format to assign to the file within the archive.  Defaults to the current time.



### vfile = archive:fileopen(fileName)

Open an existing file entry within the archive.  Returns a userdata representing the read-only file in the archive.



### archive:filecloseall()

Closes all open files within the archive.



### bool ret = archive:fileerase(filename)

Erases a file from the archive.  Returns `true` if the file was erased successfully, `false` otherwise.

Erased files do not free up their space within the archive until `archive:pack()` is called.

* `filename` is the name of the file to erase within the archive.



### bool ret = archive:filerename(oldFilename, newFilename)

Renames a file within the archive.  Returns `true` if the file was renamed successfully, `false` otherwise.

* `oldFilename` is the name of an existing file to rename.
* `newFilename` is the new name of the file.



### bool ret = archive:fileinsert(srcFilename, destFilename [, compressionMethod = ziparchive.DEFLATED] [, compressionLevel = ziparchive.DEFAULT_COMPRESSION]  [, time_t fileTime])

Inserts a file from disk into the archive.  Returns `true` if the file was renamed successfully, `false` otherwise.

* `srcFilename` is the path to a filename on disk.  This file will be transferred into the archive.
* `destFilename` is the name of the file entry within the archive that `srcFilename` is copied into.
* `compressionMethod`: See above.
* `compressionLevel`: See above.
* `fileTime` is the time in time_t format to assign to the file within the archive.  Defaults to the `srcFilename` time.



### bool ret = archive:fileextract(srcFilename, destFilename)

Extracts a file from the archive and stores it on disk.  Returns `true` if the file was renamed successfully, `false` otherwise.

* `srcFilename` is the name of the file to extract from within the archive.
* `destFilename` is the name of the file to write to disk.




### ret = archive:pack( [PackOptions] )

Tightly packs a zip archive.  If the `PackOptions` parameter is specified, the zip archive is packed according to the `PackOptions.FileOrder` table given.  Extra files not specified in `PackOptions.FileOrder` are just appended out of order at the end of the archive.  Returns `true` if successful, `false` otherwise.

    PackOptions = {
        FileOrder = {
            FileOrderInfo,
            FileOrderInfo,
            .
            .
            .
        },
    }

    FileOrderInfo = {
        string EntryName,   -- The file entry name within the archive.  If a | (pipe) is specified, the left side is the full path to an archive, and the right side is the entry within the archive.
        string SourcePath,  -- The source path on disk of the file to insert into the archive
		CompressionMethod,  -- See above.
		CompressionLevel,   -- See above.
        boolean Compressed, -- true (default) if the file should be compressed, false for uncompressed
        time_t Timestamp,   -- The timestamp in time_t form.  0 (default) means the timestamp is retrieved from SourcePath
        integer Size,       -- The size of the file.  0 (default) means the size is retrieved from SourcePath
        uint32_t CRC,       -- The CRC of the file.  0 (default) causes the CRC to be calculated from SourcePath
        string MD5,         -- The MD5 of the file.  0 (default) causes the MD5 to be calculated from SourcePath
    }

Providing the fields `FileOrderInfo.Timestamp`, `FileOrderInfo.Size`, `FileOrderInfo.CRC`, and `FileOrderInfo.MD5` can help overall pack performance.  If the timestamp, size, CRC, and MD5 are already known, then `archive:pack()` won't have to calculate them.




### ret = archive:processfilelist(fileOrderTable [, ProcessFileListOptions])

The archive is rearranged according to the order of the files in `fileOrderTable`.

* `fileOrderTable` is a list of `FileOrderInfo` tables as described in `archive:pack()`.
* `ProcessFileListOptions` is an optional table giving additional ways to interact with `archive:processfilelist()`.  All fields are optional.

    ProcessFileListOptions = {
        -- If true, archive:processfilelist doesn't manipulate the archive.  Only checks to see if the archive would change.
        boolean CheckOnly,

        -- RequiresPack - If true, the archive is always repacked on change.
        boolean RequiresPack,

        -- FileCache is the path to a network file cache location to grab precompressed file entries from.
        string FileCache,

        -- FileCacheSizeThreshold is the lower file size limit before going to the network file cache.
        integer FileCacheSizeThreshold,

        -- Given a sourcePath, RetrieveChecksum returns both CRC and MD5 of the file.
        crc, md5 = function RetrieveChecksum(sourcePath),

        -- StatusUpdate is frequently called when events happen within `archive:processfilelist`.
        function StatusUpdate(FileListStatus, text),
    }
    
Enumerations for the `ProcessFileListOptions.StatusUpdate` `FileListStatus` are as follows:

* `ziparchive.UPDATING_ARCHIVE` - The archive is being updated.  This is called once per `archive:processfilelist()` call.  If the archive is never updated, then `ziparchive.UPDATING_ARCHIVE` is not fired through `StatusUpdate`.
* `ziparchive.DELETING_ENTRY` - An existing file entry was not specified in the `fileOrderTable` and is being erased.
* `ziparchive.DIRECT_COPY_FROM_ANOTHER_ARCHIVE` - A direct copy from the archive specified in `FileOrderInfo.SourcePath`.
* `ziparchive.COPYING_ENTRY_FROM_CACHE` - A direct copy of the compressed entry from the file cache is inserted into the archive.
* `ziparchive.COPYING_ENTRY_TO_CACHE` - The compressed file is copied to the cache.
* `ziparchive.UPDATING_ENTRY` - The entry is being updated within the archive.
* `ziparchive.PACKING_ARCHIVE` - The archive is being packed.




### string fileName = archive:getfilename()

Returns the filename of the archive.



### int entryCount = archive:fileentrycount()

Returns the count of file entries within the archive.



### FileEntryInfo fileEntry = archive:fileentry(index)

Returns the file entry at `index`.

    FileEntryInfo = {
        string filename,           -- The filename of the file entry.
        time_t timestamp,          -- The timestamp of the file entry.
        integer crc,               -- The CRC of the file entry.
        string md5,                -- The MD5 of the file entry.
        integer offset,            -- The offset of the file entry.  Not generally interesting to the end user.
        integer uncompressed_size, -- The uncompressed size of the file entry.
        integer compressed_size,   -- The compressed size of the file entry.
        integer compression_method,-- The compression method of the file entry, 0 for uncompressed, 8 for compressed.
    }

Two additional fields exist within the `FileEntryInfo` table.

* `FileEntryInfo.table` returns a full `FileEntryInfo` table as described above.
* `FileEntryInfo.contents` contains the uncompressed contents of the file.



### FileEntryInfo fileEntry = archive:findfileentry(filename)

Returns the file entry matching `filename`.



### int entryIndex = archive:fileentryindex(filename)

Looks up the file entry by `filename` and returns the file entry index.  Returns -1 if not found.



### for entry in archive:files()

`archive:files()` returns an iterator for the archive file entries.

    for entry in archive:files() do
        print(entry)
    end



## File commands:

### string filename = archive:filegetfilename(vfile)

Returns the filename of the current file entry.



### long position = archive:filegetposition(vfile)

Returns the current position of the file entry.



### archive:filesetlength(vfile, newLength)

Sets the `newLength` of the `vfile`.




### long len = archive:filegetlength(vfile)

Returns the length of the current file entry.



### buffer = archive:fileread(vfile [, size])

Reads `size` bytes from the current file entry.

* `vfile` - A file returned by `archive:fileopen()`.
* `size` - If not specified, all bytes are read from the file.  Otherwise, `size` bytes are read.


### archive:filewrite(vfile, buffer [, size])

Writes `size` bytes from `buffer` into the current file entry.

* `vfile` - A file returned by `archive:filecreate()`.
* `buffer` - The buffer to write to the file.
* `size` - If not specified, all bytes are written to the file.  Otherwise, `size` bytes are written.


### archive:close(vfile)

Closes the current file entry.


