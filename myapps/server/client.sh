#!/usr/bin/ruby -w

require 'net/http'

url = URI.parse('http://192.168.33.10:2345/index.html')

req = Net::HTTP::Get.new(url.to_s)

res = Net::HTTP.start(url.host, url.port) {|http|
    http.request(req)
}

puts res.body

url = URI.parse('http://192.168.33.10:2345/frap.png')

req = Net::HTTP::Get.new(url.to_s)

res = Net::HTTP.start(url.host, url.port) {|http|
http.request(req)
}

puts res.body

url = URI.parse('http://192.168.33.10:2345/index.html')

req = Net::HTTP::Get.new(url.to_s)

res = Net::HTTP.start(url.host, url.port) {|http|
http.request(req)
}

puts res.body

url = URI.parse('http://192.168.33.10:2345/index.html')

req = Net::HTTP::Get.new(url.to_s)

res = Net::HTTP.start(url.host, url.port) {|http|
http.request(req)
}

puts res.body

url = URI.parse('http://192.168.33.10:2345/frap.png')

req = Net::HTTP::Get.new(url.to_s)

res = Net::HTTP.start(url.host, url.port) {|http|
http.request(req)
}

puts res.body

url = URI.parse('http://192.168.33.10:2345/frap.png')

req = Net::HTTP::Get.new(url.to_s)

res = Net::HTTP.start(url.host, url.port) {|http|
http.request(req)
}

puts res.body

url = URI.parse('http://192.168.33.10:2345/index.html')

req = Net::HTTP::Get.new(url.to_s)

res = Net::HTTP.start(url.host, url.port) {|http|
http.request(req)
}

puts res.body