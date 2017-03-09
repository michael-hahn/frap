#!/usr/bin/ruby -w

require "socket"
require "uri"

WEB_ROOT = './public'

#map extensions to their content type
CONTENT_TYPE_MAPPING = {
	'html' => 'text/html',
	'txt' => 'text/plain',
	'png' => 'image/png',
	'jpg' => 'image/jpeg'
}

#Treat as binary data if content type cannot be found
DEFAULT_CONTENT_TYPE = 'application/octet-stream'

#This function parse the extension of the requested file and then looks up its content type
def content_type(path)
	ext = File.extname(path).split(".").last
	CONTENT_TYPE_MAPPING.fetch(ext, DEFAULT_CONTENT_TYPE)
end


#This function parses the Request-Line and generates a path to a file on the server
def requested_file(request_line)
	request_uri = request_line.split(" ")[1]
	path = URI.unescape(URI(request_uri).path)

	File.join(WEB_ROOT, path)
end


#Calling to_s creates Ruby server stackoverflow
class Abnormal
	def to_s
		puts self
	end
end


webserver = TCPServer.new('192.168.33.10', 2345)

puts "Server is up and running"

while (session = webserver.accept)
	#session.print "HTTP/1.1 200/OK\r\nContent-type:ext/html\r\n\r\n"
	
	request_line = session.gets

	STDERR.puts request_line

	path = requested_file(request_line)

	path = File.join(path, 'index.html') if File.directory?(path)

	#response = "Hello World!\n"

	#This code is injected as a fault
	if !File.exist?(path)
		Abnormal.new.to_s
	end

	if File.exist?(path) && !File.directory?(path)
		File.open(path, "rb") do |file|
			session.print  "HTTP/1.1 200 OK\r\n" + 
				"Content-Type: #{content_type(file)}\r\n" +
				"Content-Length: #{file.size}\r\n" +
				"Connection: close\r\n"
			session.print "\r\n"

			IO.copy_stream(file, session)
		end
	else
		#with wrong path, this is the correct handling
		message = "File not found\n"
		
		session.print "HTTP/1.1 404 Not Found\r\n" +
				"Content-Type: text/plain\r\n" +
				"Content-Length: #{message.size}\r\n" +
				"Connection: close\r\n"
		session.print "\r\n"

		session.print message
	end


	#session.print response
	#trimmedrequest = request.gsub(/GET\ \//, '').gsub(/\ HTTP.*/, '')
	#filename = trimmedrequest.chomp
	#if filename == ""
	#	filename = "index.html"
	#end
	#begin
	#	displayfile = File.open(filename, 'r')
	#	content = displayfile.read()
	#	session.print content
	#rescue Errno::ENOENT
	#	session.print "File not found"
	#end
	session.close
end

