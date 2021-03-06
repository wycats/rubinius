require File.dirname(__FILE__) + '/../../../spec_helper'
require 'stringio'
require 'zlib'

describe "GzipReader#read" do

  before :each do
    @data = '12345abcde'
    @zip = "\037\213\b\000,\334\321G\000\00334261MLJNI\005\000\235\005\000$\n\000\000\000"
    @io = StringIO.new @zip
  end

  it "reads the contents of a gzip file" do
    gz = Zlib::GzipReader.new @io

    gz.read.should == @data
  end

  it "reads the contents up to a certain size" do
    gz = Zlib::GzipReader.new @io

    gz.read(5).should == @data[0...5]

    gz.read(5).should == @data[5...10]
  end
  
  it "should not accept a negative length to read" do
    gz = Zlib::GzipReader.new @io
    lambda {
      gz.read(-1)
    }.should raise_error(ArgumentError)
  end
  
  it "should return an empty string if a 0 length is given" do
    gz = Zlib::GzipReader.new @io
    gz.read(0).should eql("")
  end

end
