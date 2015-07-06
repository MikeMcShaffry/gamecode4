require 'p4'
p4 = P4.P4()
p4:connect()

MyResolver = {}
function MyResolver.resolve(self, mergeData)
	print(mergeData.your_name, mergeData.their_name, mergeData.base_name,
		mergeData.your_path, mergeData.their_path, mergeData.base_path,
		mergeData.result_path, mergeData.merge_hint)
	if not mergeData.merge_hint == "e" then
		return mergeData.merge_hint
	else
		return "s" -- skip the resolve, there is a conflict
	end
end

p4:run_resolve { resolver = MyResolver }

p4:disconnect()
