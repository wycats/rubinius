require File.dirname(__FILE__) + '/../../../spec_helper'
require File.dirname(__FILE__) + '/../fixtures/classes'

describe "Socket::IPSocket#peeraddr" do

  before :each do
    @server = TCPServer.new(SocketSpecs.port)
    @client = TCPSocket.new("127.0.0.1", SocketSpecs.port)
  end

  after :each do
    @server.close unless @server.closed?
    @client.close unless @client.closed?
  end

  it "returns an array of information on the peer" do
    BasicSocket.do_not_reverse_lookup = false
    addrinfo = @client.peeraddr
    addrinfo[0].should == "AF_INET"
    addrinfo[1].should == SocketSpecs.port
    addrinfo[2].should == "localhost"
    addrinfo[3].should == "127.0.0.1"
  end

  it "returns an IP instead of hostname if do_not_reverse_lookup is true" do
    BasicSocket.do_not_reverse_lookup = true
    addrinfo = @client.peeraddr
    addrinfo[0].should == "AF_INET"
    addrinfo[1].should == SocketSpecs.port
    addrinfo[2].should == "127.0.0.1"
    addrinfo[3].should == "127.0.0.1"
  end

  it "raises error if socket is not connected" do
    lambda { @server.peeraddr }.should raise_error
  end
  
end