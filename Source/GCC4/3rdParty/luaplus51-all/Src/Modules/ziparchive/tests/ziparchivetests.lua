require 'ziparchive'

lowerAlphabet = 'abcdefghijklmnopqrstuvwxyz'

archive = ziparchive.new()
assert(archive)







-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
assert(archive:open('tempfile.zip', 'w') == true)
assert(archive:close())

assert(archive:open('tempfile.zip') == true)
assert(archive:close())

os.remove('tempfile.zip')
assert(archive:open('tempfile.zip') == false)

archive = assert(ziparchive.open('tempfile.zip', 'w'))
assert(archive:close())

archive = assert(ziparchive.open('tempfile.zip'))
assert(archive:close())

os.remove('tempfile.zip')
assert(ziparchive.open('tempfile.zip') == nil)



-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Test encrypted headers.
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
assert(archive:open('tempfile.zip', 'w', 'testpassword') == true)
assert(archive:close())

assert(archive:open('tempfile.zip', 'r', 'testpassword') == true)
assert(archive:close())

assert(archive:open('tempfile.zip', 'r', 'testpassword2') == false)
assert(archive:close())

assert(archive:open('tempfile.zip') == false)
assert(archive:close())

os.remove('tempfile.zip')




-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Test unencrypted writes.
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
assert(archive:open('tempfile.zip', 'w') == true)

fileHandle = archive:filecreate('unencrypted', 0)
assert(fileHandle)
assert(archive:filewrite(fileHandle, lowerAlphabet) == lowerAlphabet:len())
assert(archive:fileclose(fileHandle) == true)

fileHandle = archive:filecreate('unencrypted_compressed', 8)
assert(fileHandle)
assert(archive:filewrite(fileHandle, lowerAlphabet) == lowerAlphabet:len())
assert(archive:fileclose(fileHandle) == true)

fileHandle = archive:fileopen('unencrypted')
assert(fileHandle)
assert(archive:fileread(fileHandle) == lowerAlphabet)
assert(archive:fileclose(fileHandle))

fileHandle = archive:fileopen('unencrypted_compressed')
assert(fileHandle)
assert(archive:fileread(fileHandle) == lowerAlphabet)
assert(archive:fileclose(fileHandle))

assert(archive:close())

assert(archive:open('tempfile.zip') == true)

fileHandle = archive:fileopen('unencrypted')
assert(fileHandle)
assert(archive:fileread(fileHandle) == lowerAlphabet)
assert(archive:fileclose(fileHandle))

fileHandle = archive:fileopen('unencrypted_compressed')
assert(fileHandle)
assert(archive:fileread(fileHandle) == lowerAlphabet)
assert(archive:fileclose(fileHandle))

assert(archive:close())

os.remove('tempfile.zip')



-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Test encrypted writes.
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
assert(archive:open('tempfile.zip', 'w', 'testpassword') == true)

fileHandle = archive:filecreate('encrypted', 0)
assert(fileHandle)
assert(archive:filewrite(fileHandle, lowerAlphabet) == lowerAlphabet:len())
assert(archive:fileclose(fileHandle) == true)

fileHandle = archive:filecreate('encrypted_compressed', 8)
assert(fileHandle)
assert(archive:filewrite(fileHandle, lowerAlphabet) == lowerAlphabet:len())
assert(archive:fileclose(fileHandle) == true)

fileHandle = archive:fileopen('encrypted')
assert(fileHandle)
assert(archive:fileread(fileHandle) == lowerAlphabet)
assert(archive:fileclose(fileHandle))

fileHandle = archive:fileopen('encrypted_compressed')
assert(fileHandle)
assert(archive:fileread(fileHandle) == lowerAlphabet)
assert(archive:fileclose(fileHandle))

assert(archive:close())

assert(archive:open('tempfile.zip', 'r', 'testpassword') == true)

fileHandle = archive:fileopen('encrypted')
assert(fileHandle)
assert(archive:fileread(fileHandle) == lowerAlphabet)
assert(archive:fileclose(fileHandle))

fileHandle = archive:fileopen('encrypted_compressed')
assert(fileHandle)
assert(archive:fileread(fileHandle) == lowerAlphabet)
assert(archive:fileclose(fileHandle))

assert(archive:close())

os.remove('tempfile.zip')




-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Test unencrypted writes one byte at a time.
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
assert(archive:open('tempfile.zip', 'w') == true)

fileHandle = archive:filecreate('unencrypted', 0)
assert(fileHandle)
for index = 1, #lowerAlphabet do
	assert(archive:filewrite(fileHandle, lowerAlphabet:sub(index, index)) == 1)
end
assert(archive:fileclose(fileHandle) == true)

fileHandle = archive:filecreate('unencrypted_compressed', 8)
assert(fileHandle)
for index = 1, #lowerAlphabet do
	assert(archive:filewrite(fileHandle, lowerAlphabet:sub(index, index)) == 1)
end
assert(archive:fileclose(fileHandle) == true)

fileHandle = archive:fileopen('unencrypted')
assert(fileHandle)
for index = 1, #lowerAlphabet do
	assert(archive:fileread(fileHandle, 1) == lowerAlphabet:sub(index, index))
end
assert(archive:fileclose(fileHandle))

fileHandle = archive:fileopen('unencrypted_compressed')
assert(fileHandle)
for index = 1, #lowerAlphabet do
	assert(archive:fileread(fileHandle, 1) == lowerAlphabet:sub(index, index))
end
assert(archive:fileclose(fileHandle))

assert(archive:close())

assert(archive:open('tempfile.zip') == true)

fileHandle = archive:fileopen('unencrypted')
assert(fileHandle)
for index = 1, #lowerAlphabet do
	assert(archive:fileread(fileHandle, 1) == lowerAlphabet:sub(index, index))
end
assert(archive:fileclose(fileHandle))

fileHandle = archive:fileopen('unencrypted_compressed')
assert(fileHandle)
for index = 1, #lowerAlphabet do
	assert(archive:fileread(fileHandle, 1) == lowerAlphabet:sub(index, index))
end
assert(archive:fileclose(fileHandle))

assert(archive:close())

os.remove('tempfile.zip')



-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Test encrypted writes.
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
lowerAlphabet = 'abcdefghijklmnopqrstuvwxyz'

assert(archive:open('tempfile.zip', 'w', 'testpassword') == true)

fileHandle = archive:filecreate('encrypted', 0)
assert(fileHandle)
for index = 1, #lowerAlphabet do
	assert(archive:filewrite(fileHandle, lowerAlphabet:sub(index, index)) == 1)
end
assert(archive:fileclose(fileHandle) == true)

fileHandle = archive:filecreate('encrypted_compressed', 8)
assert(fileHandle)
for index = 1, #lowerAlphabet do
	assert(archive:filewrite(fileHandle, lowerAlphabet:sub(index, index)) == 1)
end
assert(archive:fileclose(fileHandle) == true)

fileHandle = archive:fileopen('encrypted')
assert(fileHandle)
for index = 1, #lowerAlphabet do
	assert(archive:fileread(fileHandle, 1) == lowerAlphabet:sub(index, index))
end
assert(archive:fileclose(fileHandle))

fileHandle = archive:fileopen('encrypted_compressed')
assert(fileHandle)
for index = 1, #lowerAlphabet do
	assert(archive:fileread(fileHandle, 1) == lowerAlphabet:sub(index, index))
end
assert(archive:fileclose(fileHandle))

assert(archive:close())

assert(archive:open('tempfile.zip', 'r', 'testpassword') == true)

fileHandle = archive:fileopen('encrypted')
assert(fileHandle)
for index = 1, #lowerAlphabet do
	assert(archive:fileread(fileHandle, 1) == lowerAlphabet:sub(index, index))
end
assert(archive:fileclose(fileHandle))

fileHandle = archive:fileopen('encrypted_compressed')
assert(fileHandle)
for index = 1, #lowerAlphabet do
	assert(archive:fileread(fileHandle, 1) == lowerAlphabet:sub(index, index))
end
assert(archive:fileclose(fileHandle))

assert(archive:close())

os.remove('tempfile.zip')




-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Test compression
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
assert(archive:open('tempfile.zip', 'w') == true)

fileHandle = archive:filecreate('Compressed')
assert(fileHandle)
assert(archive:filegetlength(fileHandle) == 0)

entry = archive:findfileentry('Compressed')
assert(entry)
assert(entry.filename == 'Compressed')
assert(entry.crc == 0)
assert(entry.uncompressed_size == 0)
assert(entry.compressed_size == 0)

buffer = string.rep('1', 1000)
assert(archive:filewrite(fileHandle, buffer) == #buffer)
assert(archive:fileclose(fileHandle))

entry = archive:findfileentry('Compressed')
assert(entry.crc ~= 0)
assert(entry.uncompressed_size == #buffer)
assert(entry.compressed_size ~= 0)

fileHandle = archive:filecreate('Uncompressed', 0)
assert(fileHandle)
assert(archive:filegetlength(fileHandle) == 0)

entry = archive:findfileentry('Uncompressed')
assert(entry)
assert(entry.filename == 'Uncompressed')
assert(entry.crc == 0)
assert(entry.uncompressed_size == 0)
assert(entry.compressed_size == 0)

assert(archive:filewrite(fileHandle, buffer) == #buffer)
assert(archive:fileclose(fileHandle))

entry = archive:findfileentry('Uncompressed')
assert(entry.crc ~= 0)
assert(entry.uncompressed_size == #buffer)
assert(entry.compressed_size == #buffer)

-- Open the compressed file.
fileHandle = archive:fileopen('Compressed')
assert(fileHandle)
assert(archive:filegetlength(fileHandle) == #buffer)
assert(archive:fileread(fileHandle) == buffer)
assert(archive:fileclose(fileHandle) == true)

assert(archive:close())

-------------------------------------------------------------------------------
assert(archive:open('tempfile.zip'))
assert(archive:fileentrycount() == 2)
entry = archive:fileentry(1)
assert(entry)
assert(entry.filename == 'Compressed')
entry = archive:fileentry(2)
assert(entry)
assert(entry.filename == 'Uncompressed')
entry = nil
entry = archive:findfileentry('uncompressed')
assert(entry)
assert(entry.filename == 'Uncompressed')

-- We're read only.
assert(archive:fileerase("compressed") == false)

-- Reopen.
assert(archive:open('tempfile.zip', 'a') == true)
assert(archive:fileerase('compressed') == true)
assert(archive:fileentrycount() == 1)
entry = archive:fileentry(1)
assert(entry)
assert(entry.filename == 'Uncompressed')

assert(archive:fileopen('Compressed') == nil)

assert(archive:close())


-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Lots of files
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Two modes.  One without a password, one with.
for pass = 1, 2 do
	archive = ziparchive.new()
	local password
	if pass == 2 then password = 'testpassword' end
	assert(archive:open('tempfile.zip', 'w', password) == true)

	-- Create 1000 files.
	for index = 0, 999 do
		local fileHandle = archive:filecreate(('File%04u'):format(index))
		assert(fileHandle)
		assert(archive:fileclose(fileHandle) == true)
	end

	assert(archive:close() == true)

	assert(archive:open('tempfile.zip', 'a', password) == true)
	assert(archive:fileentrycount() == 1000)

	-- Check all the entries.
	for index = 0, archive:fileentrycount() - 1 do
		local fileEntry = archive:fileentry(index + 1)
		assert(fileEntry)

		local fileName = ('File%04u'):format(index)
		assert(fileEntry.filename == fileName)
		assert(fileEntry.uncompressed_size == 0)
	end

	for index = 0, archive:fileentrycount() - 1 do
		local fileName = ('File%04u'):format(index)
		local fileEntry = archive:findfileentry(fileName)
		assert(fileEntry)

		fileName = ('fIlE%04u'):format(index)
		fileEntry = archive:findfileentry(fileName)
		assert(fileEntry)
	end

	-- Remove a bunch of files
	for i = 0, 499 do
		local fileName = ('fIlE%04u'):format(i * 2)
		assert(archive:fileerase(fileName))
		assert(archive:fileentrycount() == 1000 - i - 1)
	end

	-- Remove the last one.
	archive:fileerase('File0999')
	assert(archive:fileentrycount() == 499)

	for i = 0, archive:fileentrycount() - 1 do
		local fileName = ("fIlE%04u"):format(i * 2 + 1)
		local fileEntry = archive:fileentry(i + 1)
		assert(fileEntry);
		assert(fileName:lower() == fileEntry.filename:lower())

		fileEntry = archive:findfileentry(fileName)
		assert(fileEntry)
	end

	-- Test the renaming API.  Do the first file in the .zip.
	assert(archive:filerename("File0001", "Longer0001") == true)
	assert(not archive:findfileentry("File0001"))
	assert(archive:findfileentry("Longer0001"))
	assert(archive:findfileentry("File0003"))

	-- Do the last file in the .zip.
	assert(archive:filerename("File0997", "Longer0997") == true)
	assert(not archive:findfileentry("File0997"))
	assert(archive:findfileentry("Longer0997"))
	assert(archive:findfileentry("File0995"))

	assert(archive:fileentrycount() == 499)

	-- Do one in the middle.
	assert(archive:filerename("File0991", "File_0991") == true)
	assert(not archive:findfileentry("File0997"))
	assert(archive:findfileentry("File_0991"))
	assert(archive:findfileentry("File0989"))
	assert(archive:findfileentry("File0993"))

	for i = 1, archive:fileentrycount() - 5 do
		local fileName = ("File%04u"):format(i * 2 + 1)
		local fileEntry = archive:fileentry(i + 1)
		assert(fileEntry)
		assert(fileName == fileEntry.filename)

		fileEntry = archive:findfileentry(fileName)
		assert(fileEntry)
	end

	assert(archive:close())
end


os.remove('tempfile.zip')


-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Process a file list into an archive.
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
function io.writeall(fileName, buffer)
	local file = io.open(fileName, 'wb')
	file:write(buffer)
	file:close()
end


io.writeall('FileA', 'abcdefg')
io.writeall('FileB', 'hijklmn')
io.writeall('FileC', 'opqrstu')

archive = ziparchive.new()
assert(archive:open('tempfile.zip', 'w') == true)

fileList = {}
fileList[#fileList + 1] = 'filea'
fileList[#fileList + 1] = './FileA'
fileList[#fileList + 1] = 8

fileList[#fileList + 1] = 'uncompressed_fileb'
fileList[#fileList + 1] = './FileB'
fileList[#fileList + 1] = 0

fileList[#fileList + 1] = 'filec'
fileList[#fileList + 1] = './FileC'
fileList[#fileList + 1] = 0

assert(archive:processfilelist(fileList, 'cache') == true)

assert(archive:close() == true)

assert(archive:open('tempfile.zip') == true)
assert(archive:fileentryindex("filea") == 1)
assert(archive:fileentryindex("uncompressed_fileb") == 2)
assert(archive:fileentryindex("filec") == 3)
assert(archive:close() == true)

os.remove('tempfile.zip')
os.remove('FileA')
os.remove('FileB')
os.remove('FileC')




-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Process a file list into an archive.
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
print("archive:processfilelist - Table description...")
io.writeall('FileA', 'abcdefg')
io.writeall('FileB', 'hijklmn')
io.writeall('FileC', 'opqrstu')

archive = ziparchive.new()
assert(archive:open('tempfile.zip', 'w') == true)

fileList = {}
fileList[#fileList + 1] = { EntryName = 'filea', SourcePath = './FileA', Compressed = true }
fileList[#fileList + 1] = { EntryName = 'uncompressed_fileb', SourcePath = './FileB', Compressed = false }
fileList[#fileList + 1] = { EntryName = 'filec', SourcePath = './FileC', Compressed = false }

assert(archive:processfilelist(fileList, 'cache') == true)
assert(archive:close() == true)

assert(archive:open('tempfile.zip') == true)
assert(archive:fileentryindex("filea") == 1)
entry = archive:fileentry(1)
assert(entry.filename == "filea")
assert(archive:fileentryindex("uncompressed_fileb") == 2)
assert(archive:fileentryindex("filec") == 3)
assert(archive:close() == true)

os.remove('tempfile.zip')
os.remove('FileA')
os.remove('FileB')
os.remove('FileC')



-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Process a file list into an archive.
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
print("archive:processfilelist - Options...")
io.writeall('FileA', 'abcdefg')
io.writeall('FileB', 'hijklmn')
io.writeall('FileC', 'opqrstu')

archive = ziparchive.new()
assert(archive:open('tempfile.zip', 'w') == true)

fileList = {}
fileList[#fileList + 1] = { EntryName = 'filea', SourcePath = './FileA', Compressed = true }
fileList[#fileList + 1] = { EntryName = 'uncompressed_fileb', SourcePath = './FileB', Compressed = false }
fileList[#fileList + 1] = { EntryName = 'filec', SourcePath = './FileC', Compressed = false }

StatusUpdate_StatusToText = {}
StatusUpdate_StatusToText[ziparchive.UPDATING_ARCHIVE] = "-> "
StatusUpdate_StatusToText[ziparchive.DELETING_ENTRY] = "\tdel: "
StatusUpdate_StatusToText[ziparchive.COPYING_ENTRY_FROM_CACHE] = "\tfrom-cache: "
StatusUpdate_StatusToText[ziparchive.COPYING_ENTRY_TO_CACHE] = "\tto-cache: "
StatusUpdate_StatusToText[ziparchive.UPDATING_ENTRY] = "\tupdate: "

fileListOptions =
{
	FileCache = 'thecache',
	StatusUpdate = function(status, text)
		io.write(StatusUpdate_StatusToText[status] .. text .. '\n')
	end,

	RetrieveChecksum = function(sourcePath)
		local crc, md5 = ziparchive.filecrcmd5(sourcePath)
		print(sourcePath, crc, md5)
		return crc, md5
	end,
}

assert(archive:processfilelist(fileList, fileListOptions) == true)
assert(archive:close() == true)

assert(archive:open('tempfile.zip') == true)
assert(archive:fileentryindex("filea") == 1)
entry = archive:fileentry(1)
assert(entry.filename == "filea")
assert(archive:fileentryindex("uncompressed_fileb") == 2)
assert(archive:fileentryindex("filec") == 3)
assert(archive:close() == true)

--os.remove('tempfile.zip')
os.remove('FileA')
os.remove('FileB')
os.remove('FileC')





