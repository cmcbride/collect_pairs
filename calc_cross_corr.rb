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
      val = 0.0 # XXX choosing to ZERO OUT this bin by default
      unless other.count[i].zero?
        val = @norm_count[i] / other.norm_count[i] - 1.0
      end

      stat.push(val, 0, @matcher[i])
    end

    stat
  end

  def integrate_by_pi(pi_max=nil)
    istat = Counts.new
    last_rp_bin = nil
    pi_bin_new = nil
    val = 0.0
    require 'pp'
    @count.size.times do |i|
      if @matcher[i].size != 2
        raise "Can only integrate 2nd binned dimension: #{@matcher[i].size} not supported"
      end
      rp_bin, pi_bin = @matcher[i]
      if( rp_bin != last_rp_bin )
        istat.push(val, 0, [ rp_bin, pi_bin_new ]) unless last_rp_bin.nil?
        last_rp_bin = rp_bin
        val = 0.0
        pi_bin_new = pi_bin
      end
      if( pi_max.nil? or pi_bin[1] <= pi_max.to_f )
        pi_bin_new[1] = pi_bin[1]
        d_pi = pi_bin[1] - pi_bin[0]
        val += @count[i] * d_pi * 2.0
      end
    end
    istat
  end

  def print(file=nil, hdr=nil)
    if file.nil?
      fout = $stdout
    else
      fout = File.open(file, "w")
    end

    fout.puts hdr unless hdr.nil?

    @count.size.times do |i|
#       ma = @matcher[i].map { |p| p.map { |v| "%.4g" % v }.join(',') }.join('  ')
      ma = @matcher[i].map { |p| "%6.4g,%-6.4g" % p }.join(' ')

      fout.puts "%15.8f  %15.8f  %s" % [ @count[i], @norm_count[i], ma ]
    end

    fout.close unless file.nil?
  end
end

if $0 == __FILE__
  if ARGV.size < 2
    puts "Usage #{File.basename($0)}  QG_FILE  QR_FILE  [OUTPUT_XI] [OUTPUT_INTEGRATED] [PI_MAX]"
    exit
  end

  file1, file2 = ARGV.first(2)
  out_xi, out_wp, pi_max = ARGV[2,3]

  c1 = Counts.new(file1)
  c2 = Counts.new(file2)
  hdr = <<-END
# QG_FILE: #{file1}
# QR_FILE: #{file2}
  END

  stat = c1.calc_stat_nat(c2)
  $stderr.puts " -> #{out_xi}"
  stat.print(out_xi, hdr)

  unless out_wp.nil?
    istat = stat.integrate_by_pi(pi_max)
    istat.print(out_wp, hdr)
    $stderr.puts " -> #{out_wp}"
  end
end

