-- $Id: tmod1.lua,v 1.1 2004/03/25 19:31:05 tomas Exp $
ap.set_content_type"text/html"

ap.rputs "ap.args = {"
ap.rputs (ap.args() or '')
ap.rputs "}<br>\n"

assert (ap.setup_client_block ("REQUEST_CHUNKED_ERROR") == ap.OK)
if ap.should_client_block () ~= 0 then
	post_data = {}
	local block, err = ap.get_client_block ()
	while block do
		table.insert (post_data, block)
		block, err = ap.get_client_block ()
	end
	if err then
		error (err)
	end
	post_data = table.concat (post_data)
end

ap.rputs "post_data = {"
ap.rputs (tostring (post_data))
ap.rputs "}\n"
