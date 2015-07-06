require 'vdrive'

lowerAlphabet = 'abcdefghijklmnopqrstuvwxyz'

drive = vdrive.new()
assert(drive)







-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
assert(drive:create('tempfile.zip') == true)
assert(drive:close())

assert(drive:open('tempfile.zip') == true)
assert(drive:close())

os.remove('tempfile.zip')
assert(drive:open('tempfile.zip') == false)



-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Test encrypted headers.
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
assert(drive:create('tempfile.zip', 'testpassword') == true)
assert(drive:close())

assert(drive:open('tempfile.zip', false, 'testpassword') == true)
assert(drive:close())

assert(drive:open('tempfile.zip', false, 'testpassword2') == false)
assert(drive:close())

assert(drive:open('tempfile.zip') == false)
assert(drive:close())

os.remove('tempfile.zip')




-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Test unencrypted writes.
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
assert(drive:create('tempfile.zip') == true)

fileHandle = drive:filecreate('unencrypted', 0)
assert(fileHandle)
assert(drive:filewrite(fileHandle, lowerAlphabet) == lowerAlphabet:len())
assert(drive:fileclose(fileHandle) == true)

fileHandle = drive:filecreate('unencrypted_compressed', 8)
assert(fileHandle)
assert(drive:filewrite(fileHandle, lowerAlphabet) == lowerAlphabet:len())
assert(drive:fileclose(fileHandle) == true)

fileHandle = drive:fileopen('unencrypted')
assert(fileHandle)
assert(drive:fileread(fileHandle) == lowerAlphabet)
assert(drive:fileclose(fileHandle))

fileHandle = drive:fileopen('unencrypted_compressed')
assert(fileHandle)
assert(drive:fileread(fileHandle) == lowerAlphabet)
assert(drive:fileclose(fileHandle))

assert(drive:close())

assert(drive:open('tempfile.zip') == true)

fileHandle = drive:fileopen('unencrypted')
assert(fileHandle)
assert(drive:fileread(fileHandle) == lowerAlphabet)
assert(drive:fileclose(fileHandle))

fileHandle = drive:fileopen('unencrypted_compressed')
assert(fileHandle)
assert(drive:fileread(fileHandle) == lowerAlphabet)
assert(drive:fileclose(fileHandle))

assert(drive:close())

os.remove('tempfile.zip')



-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Test encrypted writes.
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
assert(drive:create('tempfile.zip', 'testpassword') == true)

fileHandle = drive:filecreate('encrypted', 0)
assert(fileHandle)
assert(drive:filewrite(fileHandle, lowerAlphabet) == lowerAlphabet:len())
assert(drive:fileclose(fileHandle) == true)

fileHandle = drive:filecreate('encrypted_compressed', 8)
assert(fileHandle)
assert(drive:filewrite(fileHandle, lowerAlphabet) == lowerAlphabet:len())
assert(drive:fileclose(fileHandle) == true)

fileHandle = drive:fileopen('encrypted')
assert(fileHandle)
assert(drive:fileread(fileHandle) == lowerAlphabet)
assert(drive:fileclose(fileHandle))

fileHandle = drive:fileopen('encrypted_compressed')
assert(fileHandle)
assert(drive:fileread(fileHandle) == lowerAlphabet)
assert(drive:fileclose(fileHandle))

assert(drive:close())

assert(drive:open('tempfile.zip', 'testpassword') == true)

fileHandle = drive:fileopen('encrypted')
assert(fileHandle)
assert(drive:fileread(fileHandle) == lowerAlphabet)
assert(drive:fileclose(fileHandle))

fileHandle = drive:fileopen('encrypted_compressed')
assert(fileHandle)
assert(drive:fileread(fileHandle) == lowerAlphabet)
assert(drive:fileclose(fileHandle))

assert(drive:close())

os.remove('tempfile.zip')




-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Test unencrypted writes one byte at a time.
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
assert(drive:create('tempfile.zip') == true)

fileHandle = drive:filecreate('unencrypted', 0)
assert(fileHandle)
for index = 1, #lowerAlphabet do
	assert(drive:filewrite(fileHandle, lowerAlphabet:sub(index, index)) == 1)
end
assert(drive:fileclose(fileHandle) == true)

fileHandle = drive:filecreate('unencrypted_compressed', 8)
assert(fileHandle)
for index = 1, #lowerAlphabet do
	assert(drive:filewrite(fileHandle, lowerAlphabet:sub(index, index)) == 1)
end
assert(drive:fileclose(fileHandle) == true)

fileHandle = drive:fileopen('unencrypted')
assert(fileHandle)
for index = 1, #lowerAlphabet do
	assert(drive:fileread(fileHandle, 1) == lowerAlphabet:sub(index, index))
end
assert(drive:fileclose(fileHandle))

fileHandle = drive:fileopen('unencrypted_compressed')
assert(fileHandle)
for index = 1, #lowerAlphabet do
	assert(drive:fileread(fileHandle, 1) == lowerAlphabet:sub(index, index))
end
assert(drive:fileclose(fileHandle))

assert(drive:close())

assert(drive:open('tempfile.zip') == true)

fileHandle = drive:fileopen('unencrypted')
assert(fileHandle)
for index = 1, #lowerAlphabet do
	assert(drive:fileread(fileHandle, 1) == lowerAlphabet:sub(index, index))
end
assert(drive:fileclose(fileHandle))

fileHandle = drive:fileopen('unencrypted_compressed')
assert(fileHandle)
for index = 1, #lowerAlphabet do
	assert(drive:fileread(fileHandle, 1) == lowerAlphabet:sub(index, index))
end
assert(drive:fileclose(fileHandle))

assert(drive:close())

os.remove('tempfile.zip')



-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Test encrypted writes.
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
lowerAlphabet = 'abcdefghijklmnopqrstuvwxyz'

assert(drive:create('tempfile.zip', 'testpassword') == true)

fileHandle = drive:filecreate('encrypted', 0)
assert(fileHandle)
for index = 1, #lowerAlphabet do
	assert(drive:filewrite(fileHandle, lowerAlphabet:sub(index, index)) == 1)
end
assert(drive:fileclose(fileHandle) == true)

fileHandle = drive:filecreate('encrypted_compressed', 8)
assert(fileHandle)
for index = 1, #lowerAlphabet do
	assert(drive:filewrite(fileHandle, lowerAlphabet:sub(index, index)) == 1)
end
assert(drive:fileclose(fileHandle) == true)

fileHandle = drive:fileopen('encrypted')
assert(fileHandle)
for index = 1, #lowerAlphabet do
	assert(drive:fileread(fileHandle, 1) == lowerAlphabet:sub(index, index))
end
assert(drive:fileclose(fileHandle))

fileHandle = drive:fileopen('encrypted_compressed')
assert(fileHandle)
for index = 1, #lowerAlphabet do
	assert(drive:fileread(fileHandle, 1) == lowerAlphabet:sub(index, index))
end
assert(drive:fileclose(fileHandle))

assert(drive:close())

assert(drive:open('tempfile.zip', 'testpassword') == true)

fileHandle = drive:fileopen('encrypted')
assert(fileHandle)
for index = 1, #lowerAlphabet do
	assert(drive:fileread(fileHandle, 1) == lowerAlphabet:sub(index, index))
end
assert(drive:fileclose(fileHandle))

fileHandle = drive:fileopen('encrypted_compressed')
assert(fileHandle)
for index = 1, #lowerAlphabet do
	assert(drive:fileread(fileHandle, 1) == lowerAlphabet:sub(index, index))
end
assert(drive:fileclose(fileHandle))

assert(drive:close())

os.remove('tempfile.zip')




-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Test compression
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
assert(drive:create('tempfile.zip') == true)

fileHandle = drive:filecreate('Compressed')
assert(fileHandle)
assert(drive:filegetlength(fileHandle) == 0)

entry = drive:findfileentry('Compressed')
assert(entry)
assert(entry.FileName == 'Compressed')
assert(entry.CRC == 0)
assert(entry.UncompressedSize == 0)
assert(entry.CompressedSize == 0)

buffer = string.rep('1', 1000)
assert(drive:filewrite(fileHandle, buffer) == #buffer)
assert(drive:fileclose(fileHandle))

entry = drive:findfileentry('Compressed')
assert(entry.CRC ~= 0)
assert(entry.UncompressedSize == #buffer)
assert(entry.CompressedSize ~= 0)

fileHandle = drive:filecreate('Uncompressed', 0)
assert(fileHandle)
assert(drive:filegetlength(fileHandle) == 0)

entry = drive:findfileentry('Uncompressed')
assert(entry)
assert(entry.FileName == 'Uncompressed')
assert(entry.CRC == 0)
assert(entry.UncompressedSize == 0)
assert(entry.CompressedSize == 0)

assert(drive:filewrite(fileHandle, buffer) == #buffer)
assert(drive:fileclose(fileHandle))

entry = drive:findfileentry('Uncompressed')
assert(entry.CRC ~= 0)
assert(entry.UncompressedSize == #buffer)
assert(entry.CompressedSize == #buffer)

-- Open the compressed file.
fileHandle = drive:fileopen('Compressed')
assert(fileHandle)
assert(drive:filegetlength(fileHandle) == #buffer)
assert(drive:fileread(fileHandle) == buffer)
assert(drive:fileclose(fileHandle) == true)

assert(drive:close())

-------------------------------------------------------------------------------
assert(drive:open('tempfile.zip'))
assert(drive:getfileentrycount() == 2)
entry = drive:getfileentry(1)
assert(entry)
assert(entry.FileName == 'Compressed')
entry = drive:getfileentry(2)
assert(entry)
assert(entry.FileName == 'Uncompressed')
entry = nil
entry = drive:findfileentry('uncompressed')
assert(entry)
assert(entry.FileName == 'Uncompressed')

-- We're read only.
assert(drive:fileerase("compressed") == false)

-- Reopen.
assert(drive:open('tempfile.zip', false) == true)
assert(drive:fileerase('compressed') == true)
assert(drive:getfileentrycount() == 1)
entry = drive:getfileentry(1)
assert(entry)
assert(entry.FileName == 'Uncompressed')

assert(drive:fileopen('Compressed') == nil)

assert(drive:close())


-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Lots of files
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Two modes.  One without a password, one with.
for pass = 1, 2 do
	drive = vdrive.new()
	local password
	if pass == 2 then password = 'testpassword' end
	assert(drive:create('tempfile.zip', password) == true)

	-- Create 1000 files.
	for index = 0, 999 do
		local fileHandle = drive:filecreate(('File%04u'):format(index))
		assert(fileHandle)
		assert(drive:fileclose(fileHandle) == true)
	end

	assert(drive:close() == true)

	assert(drive:open('tempfile.zip', false, password) == true)
	assert(drive:getfileentrycount() == 1000)

	-- Check all the entries.
	for index = 0, drive:getfileentrycount() - 1 do
		local fileEntry = drive:getfileentry(index + 1)
		assert(fileEntry)

		local fileName = ('File%04u'):format(index)
		assert(fileEntry.FileName == fileName)
		assert(fileEntry.UncompressedSize == 0)
	end

	for index = 0, drive:getfileentrycount() - 1 do
		local fileName = ('File%04u'):format(index)
		local fileEntry = drive:findfileentry(fileName)
		assert(fileEntry)

		fileName = ('fIlE%04u'):format(index)
		fileEntry = drive:findfileentry(fileName)
		assert(fileEntry)
	end

	-- Remove a bunch of files
	for i = 0, 499 do
		local fileName = ('fIlE%04u'):format(i * 2)
		assert(drive:fileerase(fileName))
		assert(drive:getfileentrycount() == 1000 - i - 1)
	end

	-- Remove the last one.
	drive:fileerase('File0999')
	assert(drive:getfileentrycount() == 499)

	for i = 0, drive:getfileentrycount() - 1 do
		local fileName = ("fIlE%04u"):format(i * 2 + 1)
		local fileEntry = drive:getfileentry(i + 1)
		assert(fileEntry);
		assert(fileName:lower() == fileEntry.FileName:lower())

		fileEntry = drive:findfileentry(fileName)
		assert(fileEntry)
	end

	-- Test the renaming API.  Do the first file in the .zip.
	assert(drive:filerename("File0001", "Longer0001") == true)
	assert(not drive:findfileentry("File0001"))
	assert(drive:findfileentry("Longer0001"))
	assert(drive:findfileentry("File0003"))

	-- Do the last file in the .zip.
	assert(drive:filerename("File0997", "Longer0997") == true)
	assert(not drive:findfileentry("File0997"))
	assert(drive:findfileentry("Longer0997"))
	assert(drive:findfileentry("File0995"))

	assert(drive:getfileentrycount() == 499)

	-- Do one in the middle.
	assert(drive:filerename("File0991", "File_0991") == true)
	assert(not drive:findfileentry("File0997"))
	assert(drive:findfileentry("File_0991"))
	assert(drive:findfileentry("File0989"))
	assert(drive:findfileentry("File0993"))

	for i = 1, drive:getfileentrycount() - 5 do
		local fileName = ("File%04u"):format(i * 2 + 1)
		local fileEntry = drive:getfileentry(i + 1)
		assert(fileEntry)
		assert(fileName == fileEntry.FileName)

		fileEntry = drive:findfileentry(fileName)
		assert(fileEntry)
	end

	assert(drive:close())
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

drive = vdrive.new()
assert(drive:create('tempfile.zip') == true)

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

assert(drive:processfilelist(fileList, 'cache') == true)

assert(drive:close() == true)

assert(drive:open('tempfile.zip') == true)
assert(drive:getfileentryindex("filea") == 1)
assert(drive:getfileentryindex("uncompressed_fileb") == 2)
assert(drive:getfileentryindex("filec") == 3)
assert(drive:close() == true)

os.remove('tempfile.zip')
os.remove('FileA')
os.remove('FileB')
os.remove('FileC')




-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Process a file list into an archive.
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
print("drive:processfilelist - Table description...")
io.writeall('FileA', 'abcdefg')
io.writeall('FileB', 'hijklmn')
io.writeall('FileC', 'opqrstu')

drive = vdrive.new()
assert(drive:create('tempfile.zip') == true)

fileList = {}
fileList[#fileList + 1] = { EntryName = 'filea', SourcePath = './FileA', Compressed = true }
fileList[#fileList + 1] = { EntryName = 'uncompressed_fileb', SourcePath = './FileB', Compressed = false }
fileList[#fileList + 1] = { EntryName = 'filec', SourcePath = './FileC', Compressed = false }

assert(drive:processfilelist(fileList, 'cache') == true)
assert(drive:close() == true)

assert(drive:open('tempfile.zip') == true)
assert(drive:getfileentryindex("filea") == 1)
entry = drive:getfileentry(1)
assert(entry.FileName == "filea")
assert(drive:getfileentryindex("uncompressed_fileb") == 2)
assert(drive:getfileentryindex("filec") == 3)
assert(drive:close() == true)

os.remove('tempfile.zip')
os.remove('FileA')
os.remove('FileB')
os.remove('FileC')



-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Process a file list into an archive.
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
print("drive:processfilelist - Options...")
io.writeall('FileA', 'abcdefg')
io.writeall('FileB', 'hijklmn')
io.writeall('FileC', 'opqrstu')

drive = vdrive.new()
assert(drive:create('tempfile.zip') == true)

fileList = {}
fileList[#fileList + 1] = { EntryName = 'filea', SourcePath = './FileA', Compressed = true }
fileList[#fileList + 1] = { EntryName = 'uncompressed_fileb', SourcePath = './FileB', Compressed = false }
fileList[#fileList + 1] = { EntryName = 'filec', SourcePath = './FileC', Compressed = false }

StatusUpdate_StatusToText = {}
StatusUpdate_StatusToText[vdrive.UPDATING_ARCHIVE] = "-> "
StatusUpdate_StatusToText[vdrive.DELETING_ENTRY] = "\tdel: "
StatusUpdate_StatusToText[vdrive.COPYING_ENTRY_FROM_CACHE] = "\tfrom-cache: "
StatusUpdate_StatusToText[vdrive.COPYING_ENTRY_TO_CACHE] = "\tto-cache: "
StatusUpdate_StatusToText[vdrive.UPDATING_ENTRY] = "\tupdate: "

fileListOptions =
{
	FileCache = 'thecache',
	StatusUpdate = function(status, text)
		io.write(StatusUpdate_StatusToText[status] .. text .. '\n')
	end,

	RetrieveChecksum = function(sourcePath)
		local crc, md5 = vdrive.filecrcmd5(sourcePath)
		print(sourcePath, crc, md5)
		return crc, md5
	end,
}

assert(drive:processfilelist(fileList, fileListOptions) == true)
assert(drive:close() == true)

assert(drive:open('tempfile.zip') == true)
assert(drive:getfileentryindex("filea") == 1)
entry = drive:getfileentry(1)
assert(entry.FileName == "filea")
assert(drive:getfileentryindex("uncompressed_fileb") == 2)
assert(drive:getfileentryindex("filec") == 3)
assert(drive:close() == true)

--os.remove('tempfile.zip')
os.remove('FileA')
os.remove('FileB')
os.remove('FileC')





