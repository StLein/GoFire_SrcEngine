# Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass

Get-ChildItem -Recurse -File | Where-Object {
	$_.Extension -in '.cpp', '.hpp', '.c', '.h', '.inl' -and
	$_.FullName -notmatch '\\common\\' -and
	$_.FullName -notmatch '\\Debug\\' -and
	$_.FullName -notmatch '\\Release\\'
} | ForEach-Object {


	$file = $_.FullName
	$bytes = [System.IO.File]::ReadAllBytes($file)
	$encoding = [System.Text.Encoding]::GetEncoding(1252) # 西欧编码 1252
	
	# 按 BOM 识别编码
	if ($bytes.Length -ge 2 -and $bytes[0] -eq 0xFF -and $bytes[1] -eq 0xFE) {
		$encoding = [System.Text.Encoding]::Unicode
		return
	}
	elseif ($bytes.Length -ge 2 -and $bytes[0] -eq 0xFE -and $bytes[1] -eq 0xFF) {
		$encoding = [System.Text.Encoding]::BigEndianUnicode
		return
	}
	elseif ($bytes.Length -ge 3 -and $bytes[0] -eq 0xEF -and $bytes[1] -eq 0xBB -and $bytes[2] -eq 0xBF) {
		$encoding = [System.Text.Encoding]::UTF8
		return
	}

	$text = $encoding.GetString($bytes)
	$hasEFBFBD = $false
	for ($i = 0; $i -lt $bytes.Length - 2; $i++) {
		if ($bytes[$i] -eq 0xEF -and $bytes[$i+1] -eq 0xBF -and $bytes[$i+2] -eq 0xBD) {
			$hasEFBFBD = $true
			break
		}
	}
	if ($text.Contains("©")) {
		# 直接转 UTF-8 BOM
	} elseif ($text -match "Copyright" -and $hasEFBFBD) {
		for ($i = 0; $i -lt $bytes.Length - 2; $i++) {
			if ($bytes[$i] -eq 0xEF -and $bytes[$i+1] -eq 0xBF -and $bytes[$i+2] -eq 0xBD) {
				$bytes[$i]   = 0xC2
				$bytes[$i+1] = 0xA9
				$bytes = $bytes[0..($i+1)] + $bytes[($i+3)..($bytes.Length-1)]
				break
			}
		}
		$text = [System.Text.Encoding]::UTF8.GetString($bytes)
	} else {
		# 936（GBK / 简体中文默认代码页）
		$cp936 = [System.Text.Encoding]::GetEncoding(936)

		# 用936解码 → 再重新编码成936
		$text936 = $cp936.GetString($bytes)
		$roundtrip = $cp936.GetBytes($text936)

		# 判断是否可逆
		$containsInvalidFor936 = -not [System.Collections.StructuralComparisons]::StructuralEqualityComparer.Equals($roundtrip, $bytes)
		if ($containsInvalidFor936){
			$text = [System.Text.Encoding]::UTF8.GetString($bytes)
		} else {
			return
		}
	}

	$utf8bom = New-Object System.Text.UTF8Encoding($true)
	[System.IO.File]::WriteAllText($file, $text, $utf8bom)
	Write-Host "Converte $file => UTF8 with BOM"
}