require File.dirname(__FILE__) + '/../../../spec_helper'

describe "File::Stat#<=>" do
  before :each do
    @name1 = tmp("i_exist")
    @name2 = tmp("i_exist_too")
    @file1 = File.new @name1, "w"
    @file2 = File.new @name2, "w"
  end

  after :each do
    @file1.close unless @file1.closed?
    @file2.close unless @file2.closed?
    File.delete @name1 if File.exists? @name1
    File.delete @name2 if File.exists? @name2
  end

  it "is able to compare files by the same modification times" do
    now = Time.now
    File.utime(now, now, @name1)
    File.utime(now, now, @name2)
    (@file1.stat <=> @file2.stat).should == 0
  end

  it "is able to compare files by different modification times" do
    now = Time.now
    File.utime(now, now + 100, @name2)
    (@file1.stat <=> @file2.stat).should == -1

    File.utime(now, now - 100, @name2)
    (@file1.stat <=> @file2.stat).should == 1
  end

  it "should also include Comparable and thus == shows mtime equality between two File::Stat objects" do
    (@file1.stat == @file2.stat).should == true
    (@file1.stat == @file1.stat).should == true
    (@file2.stat == @file2.stat).should == true

    now = Time.now
    File.utime(now, now + 100, @name2)

    (@file1.stat == @file2.stat).should == false
    (@file1.stat == @file1.stat).should == true
    (@file2.stat == @file2.stat).should == true
  end
end
