require 'ex'

scriptPath = ((debug.getinfo(1, "S").source:match("@(.+)[\\/]") or '.') .. '\\'):gsub('\\', '/'):lower()

function test(a, b)
	if a ~= b then
		print('Actual:', a)
		print('Expected:', b)
		assert()
	end
end

test(os.path.add_extension('', 'ext'), '.ext')
test(os.path.add_extension(' ', ' ext'), ' . ext')
test(os.path.add_extension('', '.ext'), '.ext')
test(os.path.add_extension('hello', 'ext'), 'hello.ext')
test(os.path.add_extension('hello', '.ext'), 'hello.ext')
test(os.path.add_extension('hello.txt', 'ext'), 'hello.txt.ext')
test(os.path.add_extension('hello.txt', '.ext'), 'hello.txt.ext')


test(os.path.add_slash(''), '/')
test(os.path.add_slash(' '), ' /')
test(os.path.add_slash('hello'), 'hello/')
test(os.path.add_slash(' hello'), ' hello/')
test(os.path.add_slash(' hello '), ' hello /')
test(os.path.add_slash('hello/'), 'hello/')


test(os.path.append('', 'filename.txt'), 'filename.txt')
test(os.path.append('', 'dir', 'filename.txt'), 'dir/filename.txt')
test(os.path.append('', 'dirA', '', 'dirB', 'filename.txt'), 'dirA/dirB/filename.txt')
test(os.path.append('..', 'filename.txt'), '../filename.txt')
test(os.path.append('root', 'filename.txt'), 'root/filename.txt')
test(os.path.append('root', 'dir', 'filename.txt'), 'root/dir/filename.txt')
test(os.path.append('root', 'dirA', '', 'dirB', 'filename.txt'), 'root/dirA/dirB/filename.txt')
test(os.path.append('root', 'dirA', '', 'dirB', '..', 'filename.txt'), 'root/dirA/dirB/../filename.txt')
test(os.path.append('root', 'dirA', '', '/dirB', '..', 'filename.txt'), '/dirB/../filename.txt')


test(os.path.combine('', 'filename.txt'), 'filename.txt')
test(os.path.combine('', 'dir', 'filename.txt'), 'dir/filename.txt')
test(os.path.combine('', 'dirA', '', 'dirB', 'filename.txt'), 'dirA/dirB/filename.txt')
test(os.path.combine('..', 'filename.txt'), '../filename.txt')
test(os.path.combine('root', 'filename.txt'), 'root/filename.txt')
test(os.path.combine('root', 'dir', 'filename.txt'), 'root/dir/filename.txt')
test(os.path.combine('root', 'dirA', '', 'dirB', 'filename.txt'), 'root/dirA/dirB/filename.txt')
test(os.path.combine('root', 'dirA', '', 'dirB', '..', 'filename.txt'), 'root/dirA/filename.txt')
test(os.path.combine('c:/test/test2', '..', '.'), 'c:/test/')
test(os.path.combine('c:/test/test2', '..', '.', 'filename.txt'), 'c:/test/filename.txt')
test(os.path.combine('c:/test/test2', '..'), 'c:/test/')
test(os.path.combine('c:/test/test2', '..', '..'), 'c:/')
test(os.path.combine('c:/test/test2', '..', '..', '..'), 'c:/')


test(os.path.escape(''), '')
test(os.path.escape('filename.txt'), 'filename.txt')
if os.getenv("OS") == "Windows_NT" then
	test(os.path.escape(' '), '" "')
	test(os.path.escape('file name.txt'), '"file name.txt"')
else
	test(os.path.escape(' '), '\\ ')
	test(os.path.escape('file name.txt'), 'file\\ name.txt')
end


test(os.path.exists(scriptPath .. 'pathtests.lua'), true)
test(os.path.exists(scriptPath .. 'pathtests2.lua'), false)


test(os.path.find_extension(''), nil)
test(os.path.find_extension('filename'), nil)
test(os.path.find_extension('.lua'), 1)
test(os.path.find_extension('pathtests.lua'), 10)
test(os.path.find_extension('pathtests'), nil)


test(os.path.find_filename('pathtests.lua'), 1)
test(os.path.find_filename('/pathtests'), 2)
test(os.path.find_filename('c:/pathtests'), 4)
test(os.path.find_filename('c:pathtests'), 3)


test(os.path.get_extension(''), '')
test(os.path.get_extension('filename'), '')
test(os.path.get_extension('filename.ext'), '.ext')
test(os.path.get_extension('filename.txt.ext'), '.ext')


test(os.path.get_filename(''), '')
test(os.path.get_filename('filename'), 'filename')
test(os.path.get_filename('filename.ext'), 'filename.ext')
test(os.path.get_filename('c:/directory/filename.ext'), 'filename.ext')
test(os.path.get_filename('c:/directory/'), '')


test(os.path.is_directory(''), false)
test(os.path.is_directory(scriptPath .. 'pathtests.lua'), false)
test(os.path.is_directory('.'), true)
test(os.path.is_directory('..'), true)
test(os.path.is_directory(scriptPath .. '../tests'), true)
test(os.path.is_directory(scriptPath .. '../tests/'), true)


test(os.path.is_file(''), nil)
test(os.path.is_file(scriptPath .. 'pathtests.lua'), true)
test(os.path.is_file('.'), false)
test(os.path.is_file('..'), false)
test(os.path.is_file(scriptPath .. '../tests'), false)
test(os.path.is_file(scriptPath .. '../tests/'), nil)


test(os.path.is_relative(''), true)
test(os.path.is_relative('filename.ext'), true)
test(os.path.is_relative('/filename.ext'), false)
test(os.path.is_relative('c:/filename.ext'), false)


test(os.path.is_root(''), false)
test(os.path.is_root('filename.ext'), false)
test(os.path.is_root('/filename.ext'), true)
test(os.path.is_root('c:/filename.ext'), true)


test(os.path.is_unc(''), false)
test(os.path.is_unc('filename.ext'), false)
test(os.path.is_unc('/filename.ext'), false)
test(os.path.is_unc('c:/filename.ext'), false)
test(os.path.is_unc('\\\\share'), true)
test(os.path.is_unc('//share'), true)


local cwd = os.getcwd():gsub('\\', '/')
test(os.path.make_absolute(''), cwd)
test(os.path.make_absolute('.'), cwd .. '/')
test(os.path.make_absolute('..'), cwd:match('(.+)/') .. '/')
test(os.path.make_absolute('abc'), cwd .. '/abc')


test(os.path.make_backslash(''), '')
test(os.path.make_backslash(' '), ' ')
test(os.path.make_backslash('\\\\abc'), '\\\\abc')
test(os.path.make_backslash('//abc'), '\\\\abc')
test(os.path.make_backslash('c:/abc/def/'), 'c:\\abc\\def\\')


test(os.path.make_slash(''), '')
test(os.path.make_slash(' '), ' ')
test(os.path.make_slash('\\\\abc'), '//abc')
test(os.path.make_slash('//abc'), '//abc')
test(os.path.make_slash('c:\\abc\\def\\'), 'c:/abc/def/')


do
	os.remove('out.dat')

	local file = io.open('out.dat', 'wb')
	file:write('junk')
	file:close()

	test(os.path.is_writable('out.dat'), true)
	os.chmod('out.dat', 'r')
	test(os.path.is_writable('out.dat'), false)
	os.path.make_writable('out.dat')
	test(os.path.is_writable('out.dat'), true)
end	


test(os.path.match('', ''), true)
test(os.path.match('', '*'), true)
test(os.path.match('', '*.*'), false)
test(os.path.match('', 'a*'), false)
test(os.path.match('abcdefg.txt', 'a*'), true)
test(os.path.match('abcdefg.txt', 'a*b*c?e*'), true)
test(os.path.match('abcdefg.txt', 'a*b*c?f*'), false)
test(os.path.match('abcdefg.txt', '*.'), false)
test(os.path.match('abcdefg.txt', '*.t'), false)
test(os.path.match('abcdefg.txt', '*.t*'), true)
test(os.path.match('abcdefg.txt', '*.t'), false)
test(os.path.match('abcdefg.txt', '*.*t'), true)
test(os.path.match('abcdefg.txt', '*.*x'), false)
test(os.path.match('abcdefg.txt', '*.txt'), true)


test(os.path.remove_directory(''), '')
test(os.path.remove_directory(' \t'), ' \t')
test(os.path.remove_directory('abc'), 'abc')
test(os.path.remove_directory('/abc.'), 'abc.')
test(os.path.remove_directory('/dir/abc.'), 'abc.')
test(os.path.remove_directory('c:/abc.'), 'abc.')
test(os.path.remove_directory('c:/dir/abc'), 'abc')


test(os.path.remove_extension(''), '')
test(os.path.remove_extension(' \t'), ' \t')
test(os.path.remove_extension('abc'), 'abc')
test(os.path.remove_extension('abc.'), 'abc')
test(os.path.remove_extension('abc.txt'), 'abc')
test(os.path.remove_extension('abc.txt.dat'), 'abc.txt')


test(os.path.remove_filename(''), '')
test(os.path.remove_filename(' \t'), '')
test(os.path.remove_filename('abc'), '')
test(os.path.remove_filename('/abc.'), '/')
test(os.path.remove_filename('/dir/abc.'), '/dir/')
test(os.path.remove_filename('c:/abc.'), 'c:/')
test(os.path.remove_filename('c:/dir/abc'), 'c:/dir/')


test(os.path.remove_slash(''), '')
test(os.path.remove_slash(' \t'), ' \t')
test(os.path.remove_slash('abc'), 'abc')
test(os.path.remove_slash('abc/'), 'abc')


test(os.path.replace_extension('', 'ext'), '.ext')
test(os.path.replace_extension('', '.ext'), '.ext')
test(os.path.replace_extension('hello', 'ext'), 'hello.ext')
test(os.path.replace_extension('hello', '.ext'), 'hello.ext')
test(os.path.replace_extension('hello.txt', 'ext'), 'hello.ext')
test(os.path.replace_extension('hello.txt', '.ext'), 'hello.ext')
test(os.path.replace_extension('hello.txt.dat', 'ext'), 'hello.txt.ext')
test(os.path.replace_extension('hello.txt.dat', '.ext'), 'hello.txt.ext')


test(os.path.simplify(''), '')
test(os.path.simplify('abc'), 'abc')
test(os.path.simplify('.abc'), '.abc')
test(os.path.simplify('./abc'), 'abc')
test(os.path.simplify('..abc'), '..abc')
test(os.path.simplify('../abc'), '../abc')
test(os.path.simplify('abc/////def'), 'abc/def')
test(os.path.simplify('abc/././././def'), 'abc/def')
test(os.path.simplify('c:/somedir/.././././def'), 'c:/def')
test(os.path.simplify('abc/.././././def'), 'def')
test(os.path.simplify('abc/ABC/../../../../def'), 'def')
test(os.path.simplify('c:\\abc\\ABC\\../..\\../..\\def'), 'c:/def')
test(os.path.simplify('\\\\uncserver\\pathA\\file.txt'), '\\\\uncserver/pathA/file.txt')


function compare_split(path, expectedDirname, expectedFilename)
	local actualDirname, actualFilename = os.path.split(path)
	return actualDirname == expectedDirname  and  actualFilename == expectedFilename
end

test(compare_split('', '', ''), true)
test(compare_split('abc.txt', '', 'abc.txt'), true)
test(compare_split('/', '/', ''), true)
test(compare_split('/abc', '/', 'abc'), true)
test(compare_split('/abc/', '/abc/', ''), true)
test(compare_split('c:/', 'c:/', ''), true)
test(compare_split('c:/abc', 'c:/', 'abc'), true)
test(compare_split('c:/abc/', 'c:/abc/', ''), true)


test(os.path.trim('abc.txt'), 'abc.txt')
test(os.path.trim(' abc.txt'), 'abc.txt')
test(os.path.trim('abc.txt '), 'abc.txt')
test(os.path.trim('  \t  abc.txt \t \t \t '), 'abc.txt')


test(os.path.unescape(''), '')

if os.getenv("OS") == "Windows_NT" then
	test(os.path.unescape('"'), '')
	test(os.path.unescape('""'), '')
	test(os.path.unescape('" "'), ' ')
	test(os.path.unescape('"file with spaces'), 'file with spaces')
	test(os.path.unescape('"file with spaces"'), 'file with spaces')
else
	test(os.path.unescape('"'), '"')
	test(os.path.unescape('""'), '""')
	test(os.path.unescape('"\\ "'), '" "')
	test(os.path.unescape('"file\\ with\\ spaces'), '"file with spaces')
	test(os.path.unescape('"file\\ with\\ spaces"'), '"file with spaces"')
end
