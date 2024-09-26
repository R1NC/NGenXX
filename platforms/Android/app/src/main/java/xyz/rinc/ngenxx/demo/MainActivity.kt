package xyz.rinc.ngenxx.demo

import android.content.pm.PackageManager
import android.os.Bundle
import android.util.Log
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import androidx.core.content.ContextCompat
import xyz.rinc.ngenxx.NGenXX
import xyz.rinc.ngenxx.NGenXXHelper
import xyz.rinc.ngenxx.demo.ui.theme.NGenXXTheme
import java.io.File
import java.io.FileInputStream
import java.io.FileOutputStream

class MainActivity : ComponentActivity() {

    private val reqPermissionLauncher = registerForActivityResult(
        ActivityResultContracts.RequestPermission()
    ) { isGranted ->
        if (isGranted) {
            testNGenXX()
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()

        if (ContextCompat.checkSelfPermission(this, android.Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            reqPermissionLauncher.launch(android.Manifest.permission.WRITE_EXTERNAL_STORAGE)
        } else {
            testNGenXX()
        }

        setContent {
            NGenXXTheme {
                Scaffold(modifier = Modifier.fillMaxSize()) { innerPadding ->
                    Greeting(
                        txt = "",
                        modifier = Modifier.padding(innerPadding)
                    )
                }
            }
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        NGenXX.release()
    }

    @OptIn(ExperimentalStdlibApi::class)
    private fun testNGenXX() {
        val dir = filesDir?.absolutePath ?: return
        if (!NGenXX.init(dir)) return

        NGenXX.logSetCallback {level, content ->
            Log.d("NGenXX", "$level | $content")
        }

        NGenXX.logPrint(1, "deviceType:${NGenXX.deviceType()}")
        NGenXX.logPrint(1, "deviceName:${NGenXX.deviceName()}")
        NGenXX.logPrint(1, "deviceManufacturer:${NGenXX.deviceManufacturer()}")
        NGenXX.logPrint(1, "deviceOsVersion:${NGenXX.deviceOsVersion()}")
        NGenXX.logPrint(1, "deviceCpuArch:${NGenXX.deviceCpuArch()}")

        val inputStr = "0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM<>()[]{}@#$%^&*-=+~!/|_,;:'`"
        val inputBytes = inputStr.toByteArray(Charsets.UTF_8)
        val keyStr = "MNBVCXZLKJHGFDSA"
        val keyBytes = keyStr.toByteArray(Charsets.UTF_8)
        val aesEncodedBytes = NGenXX.cryptoAesEncrypt(inputBytes, keyBytes)
        val aesDecodedBytes = NGenXX.cryptoAesDecrypt(aesEncodedBytes, keyBytes)
        val aesDecodedStr = aesDecodedBytes.toString(Charsets.UTF_8)
        NGenXX.logPrint(1,"AES->$aesDecodedStr")
        val md5 = NGenXX.cryptoHashMd5(inputBytes).toHexString()
        NGenXX.logPrint(1,"md5->$md5")
        val sha256 = NGenXX.cryptoHashSha256(inputBytes).toHexString()
        NGenXX.logPrint(1,"sha256->$sha256")
        val base64Encoded = NGenXX.cryptoBase64Encode(inputBytes)
        val base64Decoded = NGenXX.cryptoBase64Decode(base64Encoded)
        val base64DecodedStr = base64Decoded.toString(Charsets.UTF_8)
        NGenXX.logPrint(1,"base64->$base64DecodedStr")

        val luaScript = application.assets.open("biz.lua").bufferedReader().use {
            it.readText()
        }
        NGenXX.lLoadS(luaScript)

        val jsonParams = "{\"url\":\"https://rinc.xyz\", \"params\":\"p0=1&p1=2&p2=3\", \"method\":0, \"headers_v\":[\"Cache-Control: no-cache\"], \"headers_c\":1, \"timeout\":6666}"
        val rsp = NGenXX.lCall("lNetHttpRequest", jsonParams)
        /*val rsp = NGenXX.netHttpRequest("https://rinc.xyz",
            "p0=1&p1=2&p2=3",
            0,
            arrayOf("Cache-Control: no-cache"),
            5555
        )*/

        val jsonDecoder = NGenXX.jsonDecoderInit(jsonParams)
        if (jsonDecoder > 0) {
            val urlNode = NGenXX.jsonDecoderReadNode(jsonDecoder, 0, "url")
            if (urlNode > 0) {
                val url = NGenXX.jsonDecoderReadString(jsonDecoder, urlNode)
                NGenXX.logPrint(1, "url:$url")
            }
            val headersCNode = NGenXX.jsonDecoderReadNode(jsonDecoder, 0, "headers_c")
            if (headersCNode > 0) {
                val headersC = NGenXX.jsonDecoderReadNumber(jsonDecoder, headersCNode)
                NGenXX.logPrint(1, "headers_c:${headersC.toInt()}")
            }
            val headersVNode = NGenXX.jsonDecoderReadNode(jsonDecoder, 0, "headers_v")
            if (headersVNode > 0) {
                var headerNode = NGenXX.jsonDecoderReadChild(jsonDecoder, headersVNode)
                while (headerNode > 0) {
                    val header = NGenXX.jsonDecoderReadString(jsonDecoder, headerNode)
                    NGenXX.logPrint(1, "header:$header")
                    headerNode = NGenXX.jsonDecoderReadNext(jsonDecoder, headerNode)
                }
            }
            NGenXX.jsonDecoderRelease(jsonDecoder)
        }

        val kvConn = NGenXX.storeKVOpen("test")
        if (kvConn > 0) {
            NGenXX.storeKVWriteString(kvConn,"s", "NGenXX")
            val s = NGenXX.storeKVReadString(kvConn,"s")
            NGenXX.logPrint(1, "s->$s")
            NGenXX.storeKVWriteInteger(kvConn,"i", 1234567890)
            val i = NGenXX.storeKVReadInteger(kvConn,"i")
            NGenXX.logPrint(1, "i->$i")
            NGenXX.storeKVWriteFloat(kvConn,"f", 0.123456789)
            val f = NGenXX.storeKVReadFloat(kvConn,"f")
            NGenXX.logPrint(1, "f->$f")
            NGenXX.storeKVClose(kvConn)
        }

        val dbConn = NGenXX.storeSQLiteOpen("test")
        if (dbConn > 0) {
            val prepareSQL = application.assets.open("prepare_data.sql").bufferedReader().use {
                it.readText()
            }
            NGenXX.storeSQLiteExecute(dbConn, prepareSQL)
            val querySQL = application.assets.open("query.sql").bufferedReader().use {
                it.readText()
            }
            val queryResult = NGenXX.storeSQLiteQueryDo(dbConn, querySQL)
            if (queryResult > 0) {
                while (NGenXX.storeSQLiteQueryReadRow(queryResult)) {
                    val platform = NGenXX.storeSQLiteQueryReadColumnText(queryResult, "platform");
                    val vendor = NGenXX.storeSQLiteQueryReadColumnText(queryResult, "vendor");
                    NGenXX.logPrint(1,"$platform->$vendor")
                }
                NGenXX.storeSQLiteQueryDrop(queryResult)
            }
            NGenXX.storeSQLiteClose(dbConn)
        }

        assets.open("prepare_data.sql").use { zipInStream ->
            val zipFile = File(externalCacheDir, "x.zip")
            if (!zipFile.exists()) {
                zipFile.delete()
            }
            zipFile.createNewFile()
            FileOutputStream(zipFile).use { zipOutStream ->
                val zipSuccess = NGenXXHelper.zZip(NGenXX.Companion.ZipMode.Default, zipInStream, zipOutStream)
                if (zipSuccess) {
                    FileInputStream(zipFile).use { unzipInStream ->
                        val unzipFile = File(externalCacheDir, "x.txt")
                        if (!unzipFile.exists()) {
                            unzipFile.delete()
                        }
                        unzipFile.createNewFile()
                        FileOutputStream(unzipFile).use { unzipOutStream ->
                            NGenXXHelper.zUnZip(unzipInStream, unzipOutStream)
                        }
                    }
                }
            }
        }
    }
}

@Composable
fun Greeting(txt: String, modifier: Modifier = Modifier) {
    Text(
        text = txt,
        modifier = modifier
    )
}

@Preview(showBackground = true)
@Composable
fun GreetingPreview() {
    NGenXXTheme {
        Greeting("Android")
    }
}