# depends on: class.rb

class IO

  BufferSize = 8096

  class Buffer

    ##
    # Create a buffer of +size+ bytes. The buffer contains an internal Channel
    # object it uses to fill itself.
    def initialize(size)
      @data = ByteArray.new(size)
      @bytes = 0
      @characters = 0
      @encoding = :buffer

      @total = size
      @channel = Channel.new
    end

    ##
    # Block until the buffer receives more data
    def process
      @channel.receive
    end

    attr_reader :channel

    ##
    # Indicates how many bytes are left
    def unused
      @total - @bytes
    end

    ##
    # Remove +count+ bytes from the front of the buffer and return them.
    # All other bytes are moved up.
    def shift_front(count)
      count = @bytes if count > @bytes

      str = String.buffer count
      str.copy_from self, 0, count, 0

      rest = @bytes - count
      @data.move_bytes count, rest, 0
      @bytes = rest

      return str
    end

    ##
    # Empty the contents of the Buffer into a String object and return it.
    def as_str
      str = String.buffer @bytes
      str.copy_from self, 0, @bytes, 0
      @bytes = 0
      return str
    end

    def empty?
      @bytes == 0
    end

    ##
    # Indicates if the Buffer has no more room.
    def full?
      @total == @bytes
    end

    ##
    # Empty the buffer.
    def reset!
      @bytes = 0
    end

    ##
    # Fill the buffer from IO object +io+. The buffer requests +unused+
    # bytes, but may not receive that many. Any new data causes this to
    # return.
    def fill_from(io)
      Scheduler.send_on_readable @channel, io, self, unused()
      obj = @channel.receive
      if obj.kind_of? Class
        raise IOError, "error occured while filling buffer (#{obj})"
      end

      io.eof! unless obj

      return obj
    end

    def inspect # :nodoc:
      "#<IO::Buffer:0x%x total=%p bytes=%p characters=%p data=%p>" % [
        object_id, @total, @bytes, @characters, @data
      ]
    end

    ##
    # Match the buffer against Regexp +reg+, and remove bytes starting
    # at the beginning of the buffer, up to the end of where the Regexp
    # matched.
    def clip_to(reg)
      if m = reg.match(self)
        idx = m.end(0)
        return shift_front(idx)
      else
        nil
      end
    end
  end

  module Constants
    F_GETFL  = Rubinius::RUBY_CONFIG['rbx.platform.fcntl.F_GETFL']
    F_SETFL  = Rubinius::RUBY_CONFIG['rbx.platform.fcntl.F_SETFL']
    ACCMODE  = Rubinius::RUBY_CONFIG['rbx.platform.fcntl.O_ACCMODE']

    SEEK_SET = Rubinius::RUBY_CONFIG['rbx.platform.io.SEEK_SET']
    SEEK_CUR = Rubinius::RUBY_CONFIG['rbx.platform.io.SEEK_CUR']
    SEEK_END = Rubinius::RUBY_CONFIG['rbx.platform.io.SEEK_END']

    RDONLY   = Rubinius::RUBY_CONFIG['rbx.platform.file.O_RDONLY']
    WRONLY   = Rubinius::RUBY_CONFIG['rbx.platform.file.O_WRONLY']
    RDWR     = Rubinius::RUBY_CONFIG['rbx.platform.file.O_RDWR']

    CREAT    = Rubinius::RUBY_CONFIG['rbx.platform.file.O_CREAT']
    EXCL     = Rubinius::RUBY_CONFIG['rbx.platform.file.O_EXCL']
    NOCTTY   = Rubinius::RUBY_CONFIG['rbx.platform.file.O_NOCTTY']
    TRUNC    = Rubinius::RUBY_CONFIG['rbx.platform.file.O_TRUNC']
    APPEND   = Rubinius::RUBY_CONFIG['rbx.platform.file.O_APPEND']
    NONBLOCK = Rubinius::RUBY_CONFIG['rbx.platform.file.O_NONBLOCK']
    SYNC     = Rubinius::RUBY_CONFIG['rbx.platform.file.O_SYNC']

    # TODO: these flags should probably be imported from Platform
    LOCK_SH  = 0x01
    LOCK_EX  = 0x02
    LOCK_NB  = 0x04
    LOCK_UN  = 0x08
    BINARY   = 0x04
  end

  include Constants

  def self.for_fd(fd = -1, mode = nil)
    self.new(fd, mode)
  end

  def self.foreach(name, sep_string = $/, &block)
    sep_string ||= ''
    io = File.open(StringValue(name), 'r')
    sep = StringValue(sep_string)
    begin
      while(line = io.gets(sep))
        yield line
      end
    ensure
      io.close
    end
  end

  ##
  # Creates a new IO object to access the existing stream referenced by the
  # descriptor given. The stream is not copied in any way so anything done on
  # one IO will affect any other IOs accessing the same descriptor.
  #
  # The mode string given must be compatible with the original one so going
  # 'r' from 'w' cannot be done but it is possible to go from 'w+' to 'r', for
  # example (since the stream is not being "widened".)
  #
  # The initialization will verify that the descriptor given is a valid one.
  # Errno::EBADF will be raised if that is not the case. If the mode is
  # incompatible, it will raise Errno::EINVAL instead.
  def self.open(*args)
    io = self.new(*args)

    return io unless block_given?

    begin
      yield io
    ensure
      io.close rescue nil unless io.closed?
    end
  end

  def self.parse_mode(mode)
    ret = 0

    case mode[0]
    when ?r
      ret |= RDONLY
    when ?w
      ret |= WRONLY | CREAT | TRUNC
    when ?a
      ret |= WRONLY | CREAT | APPEND
    else
      raise ArgumentError, "invalid mode -- #{mode}"
    end

    return ret if mode.length == 1

    case mode[1]
    when ?+
      ret &= ~(RDONLY | WRONLY)
      ret |= RDWR
    when ?b
      ret |= BINARY
    else
      raise ArgumentError, "invalid mode -- #{mode}"
    end

    return ret if mode.length == 2

    case mode[2]
    when ?+
      ret &= ~(RDONLY | WRONLY)
      ret |= RDWR
    when ?b
      ret |= BINARY
    else
      raise ArgumentError, "invalid mode -- #{mode}"
    end

    ret
  end

  ##
  # Creates a pair of pipe endpoints (connected to each other)
  # and returns them as a two-element array of IO objects:
  # [ read_file, write_file ]. Not available on all platforms.
  #
  # In the example below, the two processes close the ends of 
  # the pipe that they are not using. This is not just a cosmetic
  # nicety. The read end of a pipe will not generate an end of
  # file condition if there are any writers with the pipe still
  # open. In the case of the parent process, the rd.read will
  # never return if it does not first issue a wr.close.
  #
  #  rd, wr = IO.pipe
  #
  #  if fork
  #    wr.close
  #    puts "Parent got: <#{rd.read}>"
  #    rd.close
  #    Process.wait
  #  else
  #    rd.close
  #    puts "Sending message to parent"
  #    wr.write "Hi Dad"
  #    wr.close
  #  end
  # produces:
  #
  #  Sending message to parent
  #  Parent got: <Hi Dad>
  def self.pipe
    lhs = IO.allocate
    rhs = IO.allocate
    out = create_pipe(lhs, rhs)
    lhs.setup
    rhs.setup
    return [lhs, rhs]
  end

  ## 
  # Runs the specified command string as a subprocess;
  # the subprocess‘s standard input and output will be
  # connected to the returned IO object. If cmd_string
  # starts with a ``-’’, then a new instance of Ruby is
  # started as the subprocess. The default mode for the
  # new file object is ``r’’, but mode may be set to any
  # of the modes listed in the description for class IO.
  #
  # If a block is given, Ruby will run the command as a
  # child connected to Ruby with a pipe. Ruby‘s end of
  # the pipe will be passed as a parameter to the block. 
  # At the end of block, Ruby close the pipe and sets $?.
  # In this case IO::popen returns the value of the block.
  # 
  # If a block is given with a cmd_string of ``-’’, the
  # block will be run in two separate processes: once in
  # the parent, and once in a child. The parent process
  # will be passed the pipe object as a parameter to the
  # block, the child version of the block will be passed
  # nil, and the child‘s standard in and standard out will
  # be connected to the parent through the pipe.
  # Not available on all platforms.
  #
  #  f = IO.popen("uname")
  #  p f.readlines
  #  puts "Parent is #{Process.pid}"
  #  IO.popen ("date") { |f| puts f.gets }
  #  IO.popen("-") {|f| $stderr.puts "#{Process.pid} is here, f is #{f}"}
  #  p $?
  # produces:
  # 
  #  ["Linux\n"]
  #  Parent is 26166
  #  Wed Apr  9 08:53:52 CDT 2003
  #  26169 is here, f is
  #  26166 is here, f is #<IO:0x401b3d44>
  #  #<Process::Status: pid=26166,exited(0)>
  def self.popen(str, mode = "r")
    if str == "+-+" and !block_given?
      raise ArgumentError, "this mode requires a block currently"
    end

    mode = parse_mode mode

    readable = false
    writable = false

    if mode & IO::RDWR != 0 then
      readable = true
      writable = true
    elsif mode & IO::WRONLY != 0 then
      writable = true
    else # IO::RDONLY
      readable = true
    end

    pa_read, ch_write = IO.pipe if readable
    ch_read, pa_write = IO.pipe if writable

    pid = Process.fork do
      if readable then
        pa_read.close
        STDOUT.reopen ch_write
      end

      if writable then
        pa_write.close
        STDIN.reopen ch_read
      end

      if str == "+-+"
        yield nil
      else
        Process.replace "/bin/sh", ["sh", "-c", str]
      end
    end

    ch_write.close if readable
    ch_read.close  if writable

    # See bottom for definition
    pipe = IO::BidirectionalPipe.new pid, pa_read, pa_write

    if block_given? then
      begin
        yield pipe
      ensure
        pipe.close
      end
    else
      return pipe
    end
  end

  ##
  # Opens the file, optionally seeks to the given offset,
  # then returns length bytes (defaulting to the rest of
  # the file). read ensures the file is closed before returning.
  #
  #  IO.read("testfile")           #=> "This is line one\nThis is line two\nThis is line three\nAnd so on...\n"
  #  IO.read("testfile", 20)       #=> "This is line one\nThi"
  #  IO.read("testfile", 20, 10)   #=> "ne one\nThis is line "
  def self.read(name, length = Undefined, offset = 0)
    name = StringValue(name)
    length ||= Undefined
    offset ||= 0

    offset = Type.coerce_to(offset, Fixnum, :to_int)

    if offset < 0
      raise Errno::EINVAL, "offset must not be negative"
    end

    unless length.equal?(Undefined)
      length = Type.coerce_to(length, Fixnum, :to_int)

      if length < 0
        raise ArgumentError, "length must not be negative"
      end
    end

    File.open(name) do |f|
      f.seek(offset) unless offset.zero?

      if length.equal?(Undefined)
        f.read
      else
        f.read(length)
      end
    end
  end

  ## 
  # Reads the entire file specified by name as individual
  # lines, and returns those lines in an array. Lines are
  # separated by sep_string.
  #
  #  a = IO.readlines("testfile")
  #  a[0]   #=> "This is line one\n"
  def self.readlines(name, sep_string = $/)
    io = File.open(StringValue(name), 'r')
    return if io.nil?

    begin
      io.readlines(sep_string)
    ensure
      io.close
    end
  end

  ##
  # Select() examines the I/O descriptor sets who are passed in
  # +read_array+, +write_array+, and +error_array+ to see if some of their descriptors are
  # ready for reading, are ready for writing, or have an exceptions pending.
  #
  # If +timeout+ is not nil, it specifies a maximum interval to wait
  # for the selection to complete. If timeout is nil, the select
  # blocks indefinitely.
  # 
  # +write_array+, +error_array+, and +timeout+ may be left as nil if they are
  # unimportant
  def self.select(read_array, write_array = nil, error_array = nil,
                  timeout = nil)
    chan = Channel.new

    if read_array then
      read_array.each do |readable|
        Scheduler.send_on_readable chan, readable, nil, nil
      end
    end

    raise NotImplementedError, "write_array is not supported" if write_array
    raise NotImplementedError, "error_array is not supported" if error_array

    # HACK can't do this yet
    #if write_array then
    #  write_array.each do |writable|
    #    Scheduler.send_on_writable chan, writable, nil, nil
    #  end
    #end
    #
    #if error_array then
    #  error_array.each do |errorable|
    #    Scheduler.send_on_error chan, errorable, nil, nil
    #  end
    #end

    Scheduler.send_in_microseconds chan, (timeout * 1_000_000).to_i, nil if timeout

    value = chan.receive

    return nil if value == 1 # timeout

    io = read_array.find { |readable| readable.fileno == value }

    return nil if io.nil?

    [[io], [], []]
  end

  ##
  # Opens the given path, returning the underlying file descriptor as a Fixnum.
  #  IO.sysopen("testfile")   #=> 3
  def self.sysopen(path, mode = "r", perm = 0666)
    if mode.kind_of?(String)
      mode = parse_mode(mode)
    end

    return open_with_mode(path, mode, perm)
  end

  def initialize(fd, mode = nil)
    fd = Type.coerce_to fd, Integer, :to_int

    # Descriptor must be an open and valid one
    raise Errno::EBADF, "Invalid descriptor #{fd}" if fd < 0

    cur_mode = Platform::POSIX.fcntl(fd, F_GETFL, 0)
    raise Errno::EBADF, "Invalid descriptor #{fd}" if cur_mode < 0

    unless mode.nil?
      # Must support the desired mode.
      # O_ACCMODE is /undocumented/ for fcntl() on some platforms
      # but it should work. If there is a problem, check it though.
      new_mode = IO.parse_mode(mode) & ACCMODE
      cur_mode = cur_mode & ACCMODE

      if cur_mode != RDWR and cur_mode != new_mode
        raise Errno::EINVAL, "Invalid mode '#{mode}' for existing descriptor #{fd}"
      end
    end

    setup fd, mode
  end

  ##
  # Obtains a new duplicate descriptor for the current one.
  def initialize_copy(original) # :nodoc:
    @descriptor = Platform::POSIX.dup(@descriptor)
  end

  private :initialize_copy

  def setup(desc = nil, mode = nil)
    @descriptor = desc if desc
    @mode = mode if mode
    @buffer = IO::Buffer.new(BufferSize)
    @eof = false
    @lineno = 0
  end

  def <<(obj)
    write(obj.to_s)
    return self
  end

  def __ivars__ ; @__ivars__  ; end

  ##
  # Puts ios into binary mode. This is useful only in
  # MS-DOS/Windows environments. Once a stream is in
  # binary mode, it cannot be reset to nonbinary mode.
  def binmode
    # HACK what to do?
  end

  def breadall(buffer=nil)
    return "" if @eof and @buffer.empty?

    output = ''

    buf = @buffer

    while true
      bytes = buf.fill_from(self)

      if !bytes or buf.full?
        output << buf
        buf.reset!
      end

      break unless bytes
    end

    if buffer then
      buffer = StringValue buffer
      buffer.replace output
    else
      buffer = output
    end

    buffer
  end

  ##
  # Closes the read end of a duplex I/O stream (i.e., one
  # that contains both a read and a write stream, such as
  # a pipe). Will raise an IOError if the stream is not duplexed.
  #
  #  f = IO.popen("/bin/sh","r+")
  #  f.close_read
  #  f.readlines
  # produces:
  #
  #  prog.rb:3:in `readlines': not opened for reading (IOError)
  #   from prog.rb:3
  def close_read
    # TODO raise IOError if writable
    close
  end

  ##
  # Closes the write end of a duplex I/O stream (i.e., one
  # that contains both a read and a write stream, such as
  # a pipe). Will raise an IOError if the stream is not duplexed.
  #
  #  f = IO.popen("/bin/sh","r+")
  #  f.close_write
  #  f.print "nowhere"
  # produces:
  #
  #  prog.rb:3:in `write': not opened for writing (IOError)
  #   from prog.rb:3:in `print'
  #   from prog.rb:3
  def close_write
    # TODO raise IOError if readable
    close
  end

  ##
  # Returns true if ios is completely closed (for duplex
  # streams, both reader and writer), false otherwise.
  #
  #  f = File.new("testfile")
  #  f.close         #=> nil
  #  f.closed?       #=> true
  #  f = IO.popen("/bin/sh","r+")
  #  f.close_write   #=> nil
  #  f.closed?       #=> false
  #  f.close_read    #=> nil
  #  f.closed?       #=> true
  def closed?
    @descriptor == -1
  end

  def descriptor
    @descriptor
  end

  def dup
    raise IOError, "closed stream" if closed?
    super
  end

  ##
  # Executes the block for every line in ios, where
  # lines are separated by sep_string. ios must be
  # opened for reading or an IOError will be raised.
  #
  #  f = File.new("testfile")
  #  f.each {|line| puts "#{f.lineno}: #{line}" }
  # produces:
  #
  #  1: This is line one
  #  2: This is line two
  #  3: This is line three
  #  4: And so on...
  def each(sep=$/)
    while line = gets_helper(sep)
      yield line
    end
  end

  alias_method :each_line, :each

  def each_byte
    yield getc until eof?

    self
  end

  ##
  # Set the pipe so it is at the end of the file
  def eof!
    @eof = true
  end

  ##
  # Returns true if ios is at end of file that means
  # there are no more data to read. The stream must be
  # opened for reading or an IOError will be raised.
  #
  #  f = File.new("testfile")
  #  dummy = f.readlines
  #  f.eof   #=> true
  # If ios is a stream such as pipe or socket, IO#eof? 
  # blocks until the other end sends some data or closes it.
  #
  #  r, w = IO.pipe
  #  Thread.new { sleep 1; w.close }
  #  r.eof?  #=> true after 1 second blocking
  #
  #  r, w = IO.pipe
  #  Thread.new { sleep 1; w.puts "a" }
  #  r.eof?  #=> false after 1 second blocking
  #
  #  r, w = IO.pipe
  #  r.eof?  # blocks forever
  # Note that IO#eof? reads data to a input buffer. So IO#sysread doesn‘t work with IO#eof?.
  def eof?
    read 0 # HACK force check
    @eof and @buffer.empty?
  end

  alias_method :eof, :eof?

  ##
  # Provides a mechanism for issuing low-level commands to
  # control or query file-oriented I/O streams. Arguments
  # and results are platform dependent. If arg is a number,
  # its value is passed directly. If it is a string, it is
  # interpreted as a binary sequence of bytes (Array#pack
  # might be a useful way to build this string). On Unix
  # platforms, see fcntl(2) for details. Not implemented on all platforms.
  def fcntl(command, arg=0)
    raise IOError, "closed stream" if closed?
    if arg.kind_of? Fixnum then
      Platform::POSIX.fcntl(descriptor, command, arg)
    else
      raise NotImplementedError, "cannot handle #{arg.class}"
    end
  end

  ##
  # Returns an integer representing the numeric file descriptor for ios.
  #
  #  $stdin.fileno    #=> 0
  #  $stdout.fileno   #=> 1
  def fileno
    raise IOError, "closed stream" if closed?
    @descriptor
  end

  alias_method :to_i, :fileno

  ##
  # Flushes any buffered data within ios to the underlying
  # operating system (note that this is Ruby internal 
  # buffering only; the OS may buffer the data as well).
  #
  #  $stdout.print "no newline"
  #  $stdout.flush
  # produces:
  #
  #  no newline
  def flush
    raise IOError, "closed stream" if closed?
    true
  end

  ##
  # Immediately writes all buffered data in ios to disk. Returns
  # nil if the underlying operating system does not support fsync(2).
  # Note that fsync differs from using IO#sync=. The latter ensures
  # that data is flushed from Ruby‘s buffers, but doesn‘t not guarantee
  # that the underlying operating system actually writes it to disk.
  def fsync
    raise IOError, 'closed stream' if closed?

    err = Platform::POSIX.fsync @descriptor

    Errno.handle 'fsync(2)' if err < 0

    err
  end

  ##
  # Gets the next 8-bit byte (0..255) from ios.
  # Returns nil if called at end of file.
  #
  #  f = File.new("testfile")
  #  f.getc   #=> 84
  #  f.getc   #=> 104
  def getc
    char = read 1
    return nil if char.nil?
    char[0]
  end

  ##
  # Reads the next ``line’’ from the I/O stream;
  # lines are separated by sep_string. A separator
  # of nil reads the entire contents, and a zero-length 
  # separator reads the input a paragraph at a time (two
  # successive newlines in the input separate paragraphs).
  # The stream must be opened for reading or an IOError
  # will be raised. The line read in will be returned and
  # also assigned to $_. Returns nil if called at end of file.
  #
  #  File.new("testfile").gets   #=> "This is line one\n"
  #  $_                          #=> "This is line one\n"
  def gets(sep=$/)
    @lineno += 1

    line = gets_helper sep
    line.taint unless line.nil?

    $_ = line
    $. = @lineno

    line
  end

  ##
  #--
  # Several methods use similar rules for reading strings from IO, but differ
  # slightly. This helper is an extraction of the code.

  def gets_helper(sep=$/)
    raise IOError, "closed stream" if closed?
    return nil if @eof and @buffer.empty?

    return breadall() unless sep

    buf = @buffer

    if sep.empty?
      return gets_stripped($/ + $/)
    end

    reg = /#{sep}/m

    if str = buf.clip_to(reg)
      return str
    end

    # Do an initial fill.
    return nil if !buf.fill_from(self) and buf.empty?

    output = nil
    while true
      if str = buf.clip_to(reg)
        if output
          return output + str
        else
          return str
        end
      end

      if !buf.fill_from(self)
        if buf.empty?
          rest = nil
        else
          rest = buf.as_str
        end

        if output
          if rest
            return output << buf.as_str
          else
            return output
          end
        else
          return rest
        end
      end

      if buf.full?
        if output
          output << buf
          buf.reset!
        else
          output = buf.as_str
        end
      end
    end
  end

  def gets_stripped(sep)
    buf = @buffer

    if m = /^\n+/m.match(buf)
      buf.shift_front(m.end(0)) if m.begin(0) == 0
    end

    str = gets_helper(sep)

    if m = /^\n+/m.match(buf)
      buf.shift_front(m.end(0)) if m.begin(0) == 0
    end

    return str
  end
  
  ##
  # Return a string describing this IO object.
  def inspect
    "#<#{self.class}:0x#{object_id.to_s(16)}>"
  end

  ##
  # Returns the current line number in ios. The
  # stream must be opened for reading. lineno
  # counts the number of times gets is called,
  # rather than the number of newlines encountered.
  # The two values will differ if gets is called with
  # a separator other than newline. See also the $. variable.
  #
  #  f = File.new("testfile")
  #  f.lineno   #=> 0
  #  f.gets     #=> "This is line one\n"
  #  f.lineno   #=> 1
  #  f.gets     #=> "This is line two\n"
  #  f.lineno   #=> 2
  def lineno
    raise IOError, 'closed stream' if closed?

    @lineno
  end

  ##
  # Manually sets the current line number to the
  # given value. $. is updated only on the next read.
  #
  #  f = File.new("testfile")
  #  f.gets                     #=> "This is line one\n"
  #  $.                         #=> 1
  #  f.lineno = 1000
  #  f.lineno                   #=> 1000
  #  $. # lineno of last read   #=> 1
  #  f.gets                     #=> "This is line two\n"
  #  $. # lineno of last read   #=> 1001
  def lineno=(line_number)
    raise IOError, 'closed stream' if closed?

    raise TypeError if line_number.nil?

    @lineno = Integer line_number
  end

  ##
  # FIXME
  # Returns the process ID of a child process
  # associated with ios. This will be set by IO::popen.
  #
  #  pipe = IO.popen("-")
  #  if pipe
  #    $stderr.puts "In parent, child pid is #{pipe.pid}"
  #  else
  #    $stderr.puts "In child, pid is #{$$}"
  #  end
  # produces:
  #
  #  In child, pid is 26209
  #  In parent, child pid is 26209
  def pid
    nil
  end

  ##
  # 
  def pos
    seek 0, SEEK_CUR
  end

  alias_method :tell, :pos

  ##
  # Seeks to the given position (in bytes) in ios.
  #
  #  f = File.new("testfile")
  #  f.pos = 17
  #  f.gets   #=> "This is line two\n"
  def pos=(offset)
    offset = Integer offset

    seek offset, SEEK_SET
  end

  ##
  # Writes each given argument.to_s to the stream or $_ (the result of last
  # IO#gets) if called without arguments. Appends $\.to_s to output. Returns
  # nil.
  def print(*args)
    if args.empty?
      write $_.to_s
    else
      args.each {|o| write o.to_s }
    end

    write $\.to_s
    nil
  end

  ##
  # Formats and writes to ios, converting parameters under
  # control of the format string. See Kernel#sprintf for details.
  def printf(fmt, *args)
    write Sprintf.new(fmt, *args).parse
  end

  ##
  # If obj is Numeric, write the character whose code is obj,
  # otherwise write the first character of the string
  # representation of obj to ios.
  #
  #  $stdout.putc "A"
  #  $stdout.putc 65
  # produces:
  #
  #  AA
  def putc(obj)
    byte = if obj.__kind_of__ String then
             obj[0]
           else
             Type.coerce_to(obj, Integer, :to_int) & 0xff
           end

    write byte.chr
  end

  ##
  # Writes the given objects to ios as with IO#print.
  # Writes a record separator (typically a newline)
  # after any that do not already end with a newline
  # sequence. If called with an array argument, writes 
  # each element on a new line. If called without arguments,
  # outputs a single record separator.
  #
  #  $stdout.puts("this", "is", "a", "test")
  # produces:
  #
  #  this
  #  is
  #  a
  #  test
  def puts(*args)
    if args.empty?
      write DEFAULT_RECORD_SEPARATOR
    else
      args.each do |arg|
        if arg.nil?
          str = "nil"
        elsif RecursionGuard.inspecting?(arg)
          str = "[...]"
        elsif arg.kind_of?(Array)
          RecursionGuard.inspect(arg) do
            arg.each do |a|
              puts a
            end
          end
        else
          str = arg.to_s
        end

        if str
          write str
          write DEFAULT_RECORD_SEPARATOR unless str.suffix?(DEFAULT_RECORD_SEPARATOR)
        end
      end
    end

    nil
  end

  ##
  # Reads at most length bytes from the I/O stream,
  # or to the end of file if length is omitted or is
  # nil. length must be a non-negative integer or nil.
  # If the optional buffer argument is present, it must
  # reference a String, which will receive the data.
  #
  # At end of file, it returns nil or "" depend on length.
  # ios.read() and ios.read(nil) returns "". ios.read(positive-integer) returns nil.
  #
  #  f = File.new("testfile")
  #  f.read(16)   #=> "This is line one"
  def read(size=nil, buffer=nil)
    raise IOError, "closed stream" if closed?
    return breadall(buffer) unless size

    return nil if @eof and @buffer.empty?

    buf = @buffer
    done = false

    output = ''

    needed = size

    if needed > 0 and buf.size >= needed
      output << buf.shift_front(needed)
    else
      while true
        bytes = buf.fill_from(self)

        if bytes
          done = needed - bytes <= 0
        else
          done = true
        end

        if done or buf.full?
          output << buf.shift_front(needed)
          needed = size - output.length
        end

        break if done or needed == 0
      end
    end

    if buffer then
      buffer = StringValue buffer
      buffer.replace output
    else
      buffer = output
    end

    buffer
  end

  ##
  # Reads at most maxlen bytes from ios using read(2) system
  # call after O_NONBLOCK is set for the underlying file descriptor.
  #
  # If the optional outbuf argument is present, it must reference
  # a String, which will receive the data.
  #
  # read_nonblock just calls read(2). It causes all errors read(2)
  # causes: EAGAIN, EINTR, etc. The caller should care such errors.
  #
  # read_nonblock causes EOFError on EOF.
  #
  # If the read buffer is not empty, read_nonblock reads from the
  # buffer like readpartial. In this case, read(2) is not called.
  def read_nonblock(size, buffer = nil)
    raise IOError, "closed stream" if closed?
    prim_read(size, buffer)
  end

  ##
  # Reads a character as with IO#getc, but raises an EOFError on end of file.
  def readchar
    char = getc

    raise EOFError, 'end of file reached' if char.nil?

    char
  end

  ##
  # Reads a line as with IO#gets, but raises an EOFError on end of file.
  def readline(sep=$/)
    out = gets(sep)
    raise EOFError, "end of file" unless out
    return out
  end

  ##
  # Reads all of the lines in ios, and returns them in an array.
  # Lines are separated by the optional sep_string. If sep_string
  # is nil, the rest of the stream is returned as a single record.
  # The stream must be opened for reading or an IOError will be raised.
  #
  #  f = File.new("testfile")
  #  f.readlines[0]   #=> "This is line one\n"
  def readlines(sep=$/)
    ary = Array.new
    while line = gets(sep)
      ary << line
    end
    return ary
  end

  ##
  # Reads at most maxlen bytes from the I/O stream. It blocks
  # only if ios has no data immediately available. It doesn‘t
  # block if some data available. If the optional outbuf argument
  # is present, it must reference a String, which will receive the
  # data. It raises EOFError on end of file.
  #
  # readpartial is designed for streams such as pipe, socket, tty,
  # etc. It blocks only when no data immediately available. This
  # means that it blocks only when following all conditions hold.
  #
  # the buffer in the IO object is empty.
  # the content of the stream is empty.
  # the stream is not reached to EOF.
  # When readpartial blocks, it waits data or EOF on the stream.
  # If some data is reached, readpartial returns with the data.
  # If EOF is reached, readpartial raises EOFError.
  #
  # When readpartial doesn‘t blocks, it returns or raises immediately.
  # If the buffer is not empty, it returns the data in the buffer.
  # Otherwise if the stream has some content, it returns the data in
  # the stream. Otherwise if the stream is reached to EOF, it raises EOFError.
  #
  #  r, w = IO.pipe           #               buffer          pipe content
  #  w << "abc"               #               ""              "abc".
  #  r.readpartial(4096)      #=> "abc"       ""              ""
  #  r.readpartial(4096)      # blocks because buffer and pipe is empty.
  #
  #  r, w = IO.pipe           #               buffer          pipe content
  #  w << "abc"               #               ""              "abc"
  #  w.close                  #               ""              "abc" EOF
  #  r.readpartial(4096)      #=> "abc"       ""              EOF
  #  r.readpartial(4096)      # raises EOFError
  #
  #  r, w = IO.pipe           #               buffer          pipe content
  #  w << "abc\ndef\n"        #               ""              "abc\ndef\n"
  #  r.gets                   #=> "abc\n"     "def\n"         ""
  #  w << "ghi\n"             #               "def\n"         "ghi\n"
  #  r.readpartial(4096)      #=> "def\n"     ""              "ghi\n"
  #  r.readpartial(4096)      #=> "ghi\n"     ""              ""
  # Note that readpartial behaves similar to sysread. The differences are:
  #
  # If the buffer is not empty, read from the buffer instead
  # of "sysread for buffered IO (IOError)".
  # It doesn‘t cause Errno::EAGAIN and Errno::EINTR. When readpartial
  # meets EAGAIN and EINTR by read system call, readpartial retry the system call.
  # The later means that readpartial is nonblocking-flag insensitive. It
  # blocks on the situation IO#sysread causes Errno::EAGAIN as if the fd is blocking mode.
  def readpartial(size, buffer = nil)
    raise ArgumentError, 'negative string size' unless size >= 0
    raise IOError, "closed stream" if closed?

    buffer = '' if buffer.nil?

    in_buf = @buffer.shift_front size
    size = size - in_buf.length

    in_buf << sysread(size) if size > 0

    buffer.replace in_buf

    buffer
  end

  alias_method :orig_reopen, :reopen

  ##
  # Reassociates ios with the I/O stream given in other_IO or to
  # a new stream opened on path. This may dynamically change the
  # actual class of this stream.
  #
  #  f1 = File.new("testfile")
  #  f2 = File.new("testfile")
  #  f2.readlines[0]   #=> "This is line one\n"
  #  f2.reopen(f1)     #=> #<File:testfile>
  #  f2.readlines[0]   #=> "This is line one\n"
  def reopen(other, mode = 'r')
    other = if other.respond_to? :to_io then
              other.to_io
            else
              File.new other, mode
            end

    raise IOError, 'closed stream' if other.closed?

    prim_reopen other

    self
  end

  ##
  # Positions ios to the beginning of input, resetting lineno to zero.
  #
  #  f = File.new("testfile")
  #  f.readline   #=> "This is line one\n"
  #  f.rewind     #=> 0
  #  f.lineno     #=> 0
  #  f.readline   #=> "This is line one\n"
  def rewind
    seek 0
    @lineno = 0
    @eof = false
    return 0
  end

  ##
  # Seeks to a given offset +amount+ in the stream according to the value of whence:
  #
  # IO::SEEK_CUR  | Seeks to _amount_ plus current position
  # --------------+----------------------------------------------------
  # IO::SEEK_END  | Seeks to _amount_ plus end of stream (you probably
  #               | want a negative value for _amount_)
  # --------------+----------------------------------------------------
  # IO::SEEK_SET  | Seeks to the absolute location given by _amount_
  # Example:
  #
  #  f = File.new("testfile")
  #  f.seek(-13, IO::SEEK_END)   #=> 0
  #  f.readline                  #=> "And so on...\n"
  def seek(amount, whence=SEEK_SET)
    raise IOError, "closed stream" if closed?
    # Unseek the still buffered amount
    unless @buffer.empty?
      prim_seek(-@buffer.size, SEEK_CUR)
      @buffer.reset!
      @eof = false
    end

    prim_seek amount, whence
  end

  ##
  # Returns status information for ios as an object of type File::Stat.
  #
  #  f = File.new("testfile")
  #  s = f.stat
  #  "%o" % s.mode   #=> "100644"
  #  s.blksize       #=> 4096
  #  s.atime         #=> Wed Apr 09 08:53:54 CDT 2003
  def stat
    raise IOError, "closed stream" if closed?

    File::Stat.from_fd fileno
  end

  ##
  # Returns the current ``sync mode’’ of ios. When sync mode is true,
  # all output is immediately flushed to the underlying operating
  # system and is not buffered by Ruby internally. See also IO#fsync.
  #
  #  f = File.new("testfile")
  #  f.sync   #=> false
  def sync
    raise IOError, "closed stream" if closed?
    true
  end

  ##
  #--
  # The current implementation does no write buffering, so we're always in
  # sync mode.

  def sync=(v)
    raise IOError, "closed stream" if closed?
  end

  ##
  # Reads integer bytes from ios using a low-level read and returns
  # them as a string. Do not mix with other methods that read from
  # ios or you may get unpredictable results. Raises SystemCallError 
  # on error and EOFError at end of file.
  #
  #  f = File.new("testfile")
  #  f.sysread(16)   #=> "This is line one"
  def sysread(size, buffer = nil)
    raise ArgumentError, 'negative string size' unless size >= 0
    raise IOError, "closed stream" if closed?

    buffer = "\0" * size unless buffer

    chan = Channel.new
    Scheduler.send_on_readable chan, self, buffer, size
    raise EOFError if chan.receive.nil?

    buffer
  end

  ##
  # Seeks to a given offset in the stream according to the value
  # of whence (see IO#seek for values of whence). Returns the new offset into the file.
  #
  #  f = File.new("testfile")
  #  f.sysseek(-13, IO::SEEK_END)   #=> 53
  #  f.sysread(10)                  #=> "And so on."
  def sysseek(amount, whence=SEEK_SET)
    raise IOError, "closed stream" if closed?
    Platform::POSIX.lseek(@descriptor, amount, whence)
  end

  def to_io
    self
  end

  alias_method :prim_tty?, :tty?

  ##
  # Returns true if ios is associated with a terminal device (tty), false otherwise.
  #
  #  File.new("testfile").isatty   #=> false
  #  File.new("/dev/tty").isatty   #=> true
  def tty?
    raise IOError, "closed stream" if closed?
    prim_tty?
  end

  alias_method :isatty, :tty?

  def wait_til_readable
    chan = Channel.new
    Scheduler.send_on_readable chan, self, nil, nil
    chan.receive
  end

  alias_method :prim_write, :write

  ##
  # Pushes back one character (passed as a parameter) onto ios,
  # such that a subsequent buffered read will return it. Only one
  # character may be pushed back before a subsequent read operation
  # (that is, you will be able to read only the last of several
  # characters that have been pushed back). Has no effect with
  # unbuffered reads (such as IO#sysread).
  #
  #  f = File.new("testfile")   #=> #<File:testfile>
  #  c = f.getc                 #=> 84
  #  f.ungetc(c)                #=> nil
  #  f.getc                     #=> 84
  def write(data)
    raise IOError, "closed stream" if closed?
    # If we have buffered data, rewind.
    unless @buffer.empty?
      seek 0, SEEK_CUR
    end

    data = String data

    return 0 if data.length == 0
    # HACK WTF?
    #raise IOError if (Platform::POSIX.fcntl(@descriptor, F_GETFL, 0) & ACCMODE) == RDONLY
    prim_write(data)
  end

  alias_method :syswrite, :write
  alias_method :write_nonblock, :write

end

##
# Implements the pipe returned by IO::pipe.

class IO::BidirectionalPipe < IO

  READ_METHODS = [
    :each,
    :each_line,
    :getc,
    :gets,
    :read,
    :read_nonblock,
    :readchar,
    :readline,
    :readlines,
    :readpartial,
    :sysread,
  ]

  WRITE_METHODS = [
    :<<,
    :print,
    :printf,
    :putc,
    :puts,
    :syswrite,
    :write,
    :write_nonblock,
  ]

  def initialize(pid, read, write)
    @pid = pid
    @read = read
    @write = write
  end

  def check_read
    raise IOError, 'not opened for reading' if @read.nil?
  end

  def check_write
    raise IOError, 'not opened for writing' if @write.nil?
  end

  ##
  # Closes ios and flushes any pending writes to the
  # operating system. The stream is unavailable for
  # any further data operations; an IOError is raised
  # if such an attempt is made. I/O streams are
  # automatically closed when they are claimed by
  # the garbage collector.
  #
  # If ios is opened by IO.popen, close sets $?.
  def close
    @read.close  if @read  and not @read.closed?
    @write.close if @write and not @write.closed?

    if @pid != 0 then
      Process.wait @pid

      @pid = 0
    end

    nil
  end

  def closed?
    if @read and @write then
      @read.closed? and @write.closed?
    elsif @read then
      @read.closed?
    else
      @write.closed?
    end
  end

  def close_read
    raise IOError, 'closed stream' if @read.closed?

    @read.close if @read
  end

  def close_write
    raise IOError, 'closed stream' if @write.closed?

    @write.close if @write
  end

  def method_missing(message, *args, &block)
    if READ_METHODS.include? message then
      check_read

      @read.send(message, *args, &block)
    elsif WRITE_METHODS.include? message then
      check_write

      @write.send(message, *args, &block)
    else
      super
    end
  end

  def pid
    raise IOError, 'closed stream' if closed?

    @pid
  end
end