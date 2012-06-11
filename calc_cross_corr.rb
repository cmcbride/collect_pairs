#!/usr/bin/env ruby

class Counts

  attr_reader :count, :norm_count, :matcher

  def parse_file(file)
    ct = []
    nc = []
    ma = []
    File.foreach(file) do |line|
      line.gsub!(/#.*$/,"")
      line.strip!
      next if line.empty?
      c = line.split

      r = c[0,2].map { |v| v.to_f }
      ct << r[0]
      nc << r[1]
      r = c[2..-1].map {|p| p.split(/,/).map { |v| v.to_f } }
      ma << r
    end
    [ct, nc, ma]
  end

  def initialize(file=nil)

    unless file.nil?
      r = parse_file(file)
      @count = r[0]
      @norm_count = r[1]
      @matcher = r[2]
    else
      @count = []
      @norm_count = []
      @matcher = []
    end

  end

  def push(count, ncount, ma)
    @count << count
    @norm_count << ncount
    @matcher << ma
  end

  # calculates: self / other - 1
  def calc_stat_nat(other)
    raise "Matcher's are *not* the same. Aborting!" unless @matcher == other.matcher

    stat = Counts.new

    @count.size.times do |i|
      val = @norm_count[i] / other.norm_count[i] - 1.0
      stat.push(val, 0, @matcher[i])
    end

    stat
  end

  def print(file=nil)
    if file.nil?
      fout = $stdout
    else
      fout = File.open(file, "w")
    end

    @count.size.times do |i|
      ma = @matcher[i].map { |p| p.map { |v| v.to_s }.join(',') }.join('  ')

      fout.puts "%15.8f  %15.8f  %s" % [ @count[i], @norm_count[i], ma ]
    end

    fout.close unless file.nil?
  end
end

if $0 == __FILE__
  if ARGV.size < 2
    puts "Usage #{File.basename($0)}  QG_FILE  QR_FILE  [OUTPUT_FILE]"
    exit
  end

  file1, file2 = ARGV.first(2)
  outfile = ARGV[2]

  c1 = Counts.new(file1)
  c2 = Counts.new(file2)

  stat = c1.calc_stat_nat(c2)

  stat.print(outfile)
end

