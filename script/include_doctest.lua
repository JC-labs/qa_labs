function download_progress(total, current)
    local ratio = current / total;
    ratio = math.min(math.max(ratio, 0), 1);
    local percent = math.floor(ratio * 100);
    print("Download progress (" .. percent .. "%/100%)")
  end

function include_doctest()
    if not os.isfile("external/include/doctest/doctest.h") then
        print "Failed to find doctest. Downloading the header..."
        os.mkdir("external/include/doctest")
        local result_str, response_code = http.download(
            "https://raw.githubusercontent.com/onqtam/doctest/2.4.1/doctest/doctest.h",
            "external/include/doctest/doctest.h",
            {
                progress = progress
            }
        )
        print ("File download response: " .. result_str .. " (" .. response_code .. ").")
    end
    includedirs "external/include"
end