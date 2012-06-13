#!/usr/bin/env ruby


def parse_file(file)
  c1 = []
  c2 = []
  ma = []
  File.foreach(file) do |line|
    line.gsub!(/#.*$/,"")
      line.strip!
    next if line.empty?
    c = line.split

    r = c[0,2].map { |v| v.to_f }
    c1 << r[0]
    c2 << r[1]
    ma << c[2..-1]
  end
  [c1, c2, ma]
end

def calc_errors(samp)
  njack = samp.size
  nbins = samp.first.size

  avg = Array.new(nbins) { 0.0 }
  sig = Array.new(nbins) { 0.0 }
  cov = Array.new(nbins) { Array.new(nbins) { 0.0 } }

  samp.each do |s|
    nbins.times do |i|
      avg[i] += s[i] / njack.to_f
    end
  end

  samp.each do |s|
    del = Array.new(nbins) { 0.0 }
    nbins.times do |i|
      del[i] = s[i] - avg[i]
      i.downto(0) do |k|
        cov[i][k] += del[i] * del[k]
      end
    end
  end

  norm = (njack - 1.0) / njack

  nbins.times do |i|
    sig[i] = Math.sqrt(norm * cov[i][i])
    i.downto(0) do |k|
      cov[i][k] *= norm / ( sig[i] * sig[k] )
      cov[k][i] = cov[i][k] unless i == k
    end
  end

  [avg, sig, cov]
end

if $0 == __FILE__
  if ARGV.size < 2
    puts "Usage #{File.basename($0)}  OUT_BASE  *JACKKNIFE"
    exit
  end

  OUT_BASE = ARGV.shift
  out_avg = OUT_BASE + ".avg.dat"
  out_err = OUT_BASE + ".diag"
  out_cov = OUT_BASE + ".ncov"

  rs = []
  fnames = []
  ma_prev = nil
  ARGV.each do |file|
    c1, c2, ma = parse_file(file)
    fnames << file
    unless( ma_prev.nil? or ma_prev == ma )
      $stderr.puts "#{ma.inspect} != #{ma_prev.inspect}"
      raise "Inconsistent Matcher!"
    end
    ma_prev = ma
    rs << c1
  end

  ma = ma_prev

  $stderr.puts "Read #{rs.size} files..."
  avg, sig, ncov = calc_errors(rs)


  $stderr.puts " -> #{out_avg}"
  File.open(out_avg, "w") do |fout|
    fout.puts "# average over jackknife samples"
    fout.puts "# Njack = #{rs.size}"
    fout.puts "# FILES: "
    i = 0
    fout.puts fnames.map do |f|
      i += 1
      "# %3d %s\n" % [i, f]
    end
    avg.size.times do |i|
      fout.puts "%15.8f  %15.8f  %s" % [ avg[i], 0.0, ma[i] ]
    end
  end

  $stderr.puts " -> #{out_err}"
  File.open(out_err, "w") do |fout|
    fout.puts "# diagonal errors from jackknife samples"
    fout.puts "# Njack = #{rs.size}"
    sig.size.times do |i|
      fout.puts "%15.8e" % sig[i]
    end
  end

  $stderr.puts " -> #{out_cov}"
  File.open(out_cov, "w") do |fout|
    fout.puts "# normalize covariance from jackknife samples "
    fout.puts "# Njack = #{rs.size}"
    fout.puts "# Covariance: #{ncov.size} x #{ncov.size}"
    ncov.size.times do |i|
      fout.puts ncov[i].map {|v| "%12.8f" % v }.join(" ")
    end
  end

end

