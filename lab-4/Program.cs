using System;
using System.Net;
using System.Net.Security;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

class Program
{
	static async Task Main(string[] args)
	{
		string[] urls = {
			// "http://pipe-racer.pro/api/users/picture/64f9b86308a5734caa417c39",
			// "http://pipe-racer.pro/api/users/picture/65049689145f88ce2e423260",
			// "http://pipe-racer.pro/api/users/picture/65072fa22c506606f266799f",
			// "http://pipe-racer.pro/api/users/picture/652c5b734279d8061f8ab5fe",
			// "http://pipe-racer.pro/api/users/picture/653538f14279d8061f8ae500",
			// "http://pageometry.weebly.com/",
			// "http://pageometry.weebly.com/section-21-sss-and-sas.html",
			"http://pageometry.weebly.com/uploads/3/8/7/1/3871472/fig2_2.png",
			"http://pageometry.weebly.com/uploads/3/8/7/1/3871472/fig3_1_1.png",
			"http://pageometry.weebly.com/uploads/3/8/7/1/3871472/fig3_1_2.png",
			"http://pageometry.weebly.com/uploads/3/8/7/1/3871472/fig3_1_3.png",
		};

		// Run all 3 in parallel and await all of them
		var eventDrivenTask = EventDrivenDownloadAsync(urls);
		var taskBasedTask = TaskBasedDownloadAsync(urls);
		var asyncAwaitTask = AsyncAwaitDownloadAsync(urls);

		await Task.WhenAll(eventDrivenTask, taskBasedTask, asyncAwaitTask);

		Console.WriteLine("Press any key to exit...");
	}

	static async Task EventDrivenDownloadAsync(string[] urls)
	{
		foreach (var url in urls)
		{
			var socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

			var endpoint = SocketHelper.GetEndpoint(url);

			var connectTask = SocketHelper.BeginConnectAsync(socket, endpoint);
			await connectTask;

			var sendTask = SocketHelper.SendRequestAsync(socket, url);
			await sendTask;

			var receiveTask = SocketHelper.BeginReceiveHeaderAsync(socket);
			await receiveTask;

			Console.WriteLine($"[EventDrivenDownloadAsync] Content-Length for {url}: {SocketHelper.ParseContentLength(receiveTask.Result)}");

			socket.Close();
		}
	}

	static async Task TaskBasedDownloadAsync(string[] urls)
	{
		// foreach (var url in urls)
		// {
		// 	var contentLength = await Task.Run(() =>
		// 	{
		// 		var socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
		// 		var endpoint = SocketHelper.GetEndpoint(url);

		// 		var connectTask = SocketHelper.BeginConnectAsync(socket, endpoint);
		// 		connectTask.Wait();

		// 		var sendTask = SocketHelper.SendRequestAsync(socket, url);
		// 		sendTask.Wait();

		// 		var receiveTask = SocketHelper.BeginReceiveHeaderAsync(socket);
		// 		receiveTask.Wait();

		// 		socket.Close();

		// 		return SocketHelper.ParseContentLength(receiveTask.Result);
		// 	});
		// 	Console.WriteLine($"[TaskBasedDownloadAsync] Content-Length for {url}: {contentLength}");

		// }
		List<Task> downloadTasks = new List<Task>();

		foreach (var url in urls)
		{
			downloadTasks.Add(
				Task.Run(() =>
				{
					var socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
					var endpoint = SocketHelper.GetEndpoint(url);

					var connectTask = SocketHelper.BeginConnectAsync(socket, endpoint);
					connectTask.Wait();

					var sendTask = SocketHelper.SendRequestAsync(socket, url);
					sendTask.Wait();

					var receiveTask = SocketHelper.BeginReceiveHeaderAsync(socket);
					receiveTask.Wait();

					socket.Close();

					Console.WriteLine($"[TaskBasedDownloadAsync] Content-Length for {url}: {SocketHelper.ParseContentLength(receiveTask.Result)}");
				})
			);
		}

		await Task.WhenAll(downloadTasks);
	}

	static async Task AsyncAwaitDownloadAsync(string[] urls)
	{
		// 	foreach (var url in urls)
		// 	{
		// 		var contentLength = await Task.Run(async () =>
		// 		{
		// 			var socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
		// 			var endpoint = SocketHelper.GetEndpoint(url);

		// 			await SocketHelper.BeginConnectAsync(socket, endpoint);
		// 			await SocketHelper.SendRequestAsync(socket, url);
		// 			var header = await SocketHelper.BeginReceiveHeaderAsync(socket);

		// 			socket.Close();

		// 			return SocketHelper.ParseContentLength(header);
		// 		});

		// 		Console.WriteLine($"[AsyncAwaitDownloadAsync] Content-Length for {url}: {contentLength}");
		// 	}
		List<Task> downloadTasks = new List<Task>();

		foreach (var url in urls)
		{
			downloadTasks.Add(
				Task.Run(async () =>
				{
					var socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
					var endpoint = SocketHelper.GetEndpoint(url);

					await SocketHelper.BeginConnectAsync(socket, endpoint);
					await SocketHelper.SendRequestAsync(socket, url);
					var header = await SocketHelper.BeginReceiveHeaderAsync(socket);

					socket.Close();

					Console.WriteLine($"[AsyncAwaitDownloadAsync] Content-Length for {url}: {SocketHelper.ParseContentLength(header)}");
				})
			);
		}

		await Task.WhenAll(downloadTasks);
	}
}

static class SocketHelper
{
	public static IPEndPoint GetEndpoint(string url)
	{
		// Extract host and port from the URL
		// Implementation not provided for brevity
		// You may use Uri class to parse the URL and get host and port
		// For example: var uri = new Uri(url); var host = uri.Host; var port = uri.Port;

		var uri = new Uri(url);
		var host = uri.Host;
		var port = uri.Port;

		return new IPEndPoint(Dns.GetHostAddresses(host)[0], port);
	}

	public static Task<bool> BeginConnectAsync(Socket socket, IPEndPoint endpoint)
	{
		var tcs = new TaskCompletionSource<bool>();
		socket.BeginConnect(endpoint, ar =>
		{
			try
			{
				socket.EndConnect(ar);
				tcs.SetResult(true);
			}
			catch (Exception ex)
			{
				tcs.SetException(ex);
			}
		}, null);
		return tcs.Task;
	}

	public static Task<int> SendRequestAsync(Socket socket, string url, CancellationToken cancellationToken = default)
	{
		var tcs = new TaskCompletionSource<int>();

		try
		{
			// Validate and sanitize the URL
			Uri uri = new Uri(url);

			var request = $"GET {uri.PathAndQuery} HTTP/1.1\r\nHost: {uri.Host}\r\n\r\n";
			var requestData = Encoding.ASCII.GetBytes(request);

			// Check for cancellation before starting the operation
			cancellationToken.ThrowIfCancellationRequested();

			socket.BeginSend(requestData, 0, requestData.Length, SocketFlags.None, ar =>
			{
				try
				{
					// Check for cancellation during the asynchronous operation
					cancellationToken.ThrowIfCancellationRequested();

					var bytesSent = socket.EndSend(ar);
					tcs.SetResult(bytesSent);
				}
				catch (Exception ex)
				{
					tcs.SetException(ex);
				}
			}, null);
		}
		catch (Exception ex)
		{
			tcs.SetException(ex);
		}

		return tcs.Task;
	}


	public static Task<string> BeginReceiveHeaderAsync(Socket socket)
	{
		var tcs = new TaskCompletionSource<string>();
		var headerBuffer = new byte[1024];
		socket.BeginReceive(headerBuffer, 0, headerBuffer.Length, SocketFlags.None, ar =>
		{
			try
			{
				var bytesRead = socket.EndReceive(ar);
				var header = Encoding.ASCII.GetString(headerBuffer, 0, bytesRead);
				tcs.SetResult(header);
			}
			catch (Exception ex)
			{
				tcs.SetException(ex);
			}
		}, null);
		return tcs.Task;
	}

	public static int ParseContentLength(string header)
	{
		// Extract Content-Length from the HTTP header
		// Implementation not provided for brevity
		// You may use regular expressions or string manipulation to extract the value

		// Console.WriteLine(header);

		// For example:
		var contentLengthStart = header.IndexOf("Content-Length:") + "Content-Length:".Length;
		var contentLengthEnd = header.IndexOf("\r\n", contentLengthStart);
		var contentLengthStr = header.Substring(contentLengthStart, contentLengthEnd - contentLengthStart).Trim();
		return int.Parse(contentLengthStr);
	}
}
