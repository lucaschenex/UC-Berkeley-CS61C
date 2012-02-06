/*
  CS 61C Project1: Small World

  Name: Hairan Zhu
  Login: cs61c-eu

  Name: Benjamin Han
  Login: cs61c-mm
 */

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.DataInput;
import java.io.DataOutput;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.lang.Math;
import java.util.*;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.filecache.DistributedCache;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.GenericWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Writable;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.input.TextInputFormat;
import org.apache.hadoop.mapreduce.lib.input.SequenceFileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.mapreduce.lib.output.TextOutputFormat;
import org.apache.hadoop.mapreduce.lib.output.SequenceFileOutputFormat;
import org.apache.hadoop.util.GenericOptionsParser;

public class SmallWorld {

	/** Maximum depth for any breadth-first search. */
	public static final int MAX_ITERATIONS = 20;

	/** Shares denom cmd-line arg across cluster. */
	public static final String DENOM_PATH = "denom.txt";

	/** The number of starting points. */
	public static long _origins = 0;

	/** Enums used for Hadoop counters. */
	public static enum GraphCounter {
		/** Count number of starting points. */
		ORIGINS,
		/** Count number of distances found. */
		DISTANCES_FOUND;
	}

	/** Wrapper for DistanceValue and DestinationsValue. */
	public static class EValue extends GenericWritable {

		/** Hadoop requires a default constructor for Writable. */
		public EValue() {
		}

		@Override
		protected Class<? extends Writable>[] getTypes() {
			return CLASSES;
		}

		private static final Class[] CLASSES = {DistanceValue.class, DestinationsValue.class};
	}

	/** Stores distance information. */
	public static class DistanceValue implements Writable {

		/** Holds distance. */
		private int distance;
		/** Origin id where distance is measured from. */
		private long origin;
		/** Indicates if distance information for that origin still needs to be propagated to successors. */
		private boolean needsPropagation;

		/** Hadoop requires a default constructor for Writable. */
		public DistanceValue() {
		}

		/** The DISTANCE from the ORIGINID to the key, and whether propagation from key is needed. */
		public DistanceValue(int distance, long originId, boolean needsPropagation) {
			this.distance = distance;
			this.origin = originId;
			this.needsPropagation = needsPropagation;
		}

		/** Serializes object - needed for Writable. */
		public void write(DataOutput out) throws IOException {
			out.writeInt(distance);
			out.writeLong(origin);
			out.writeBoolean(needsPropagation);
		}

		/** Deserializes object - needed for Writable. */
		public void readFields(DataInput in) throws IOException {
			distance = in.readInt();
			origin = in.readLong();
			needsPropagation = in.readBoolean();
		}

		/** Returns distance. */
		public int getDistance() {
			return distance;
		}

		/** Returns origin. */
		public long getOrigin() {
			return origin;
		}

		/** Returns whether distance has been propagated. */
		public boolean needDistancePropagated() {
			return needsPropagation;
		}
	}

	/** Stores destinations information. */
	public static class DestinationsValue implements Writable {

		/** Number of destinations. */
		private int nDestinations;
		/** Holds destination vertices. */
		private long[] destinations;

		/** Hadoop requires a default constructor for Writable. */
		public DestinationsValue() {
		}

		/** DestinationIds list destinations for the key. */
		public DestinationsValue(long[] destinationIds) {
			this.nDestinations = destinationIds.length;
			this.destinations = destinationIds;
		}

		/** Serializes object - needed for Writable. */
		public void write(DataOutput out) throws IOException {
			out.writeInt(nDestinations);
			for (long destination : destinations) {
				out.writeLong(destination);
			}
		}

		/** Deserializes object - needed for Writable. */
		public void readFields(DataInput in) throws IOException {
			nDestinations = in.readInt();
			destinations = new long[nDestinations];
			for (int i = 0; i < nDestinations; i++) {
				destinations[i] = in.readLong();
			}
		}

		/** Returns destinations. */
		public long[] getDestinations() {
			return destinations;
		}
	}

	/** Identity Map. */
	public static class LoaderMap extends
			Mapper<LongWritable, LongWritable, LongWritable, LongWritable> {

		@Override
		public void map(LongWritable key, LongWritable value, Context context)
				throws IOException, InterruptedException {
			context.write(key, value);
		}
	}

	/**
	 * Selects vertices as starting points with probability 1/denom. Those
	 * get marked with distance 0.
	 */
	public static class LoadReduce extends
			Reducer<LongWritable, LongWritable, LongWritable, EValue> {

		/** Denominator for probability. */
		public long denom;

		/*
		 * Setup is called automatically once per map task. This will read denom
		 * in from the DistributedCache, and it will be available to each call
		 * of map later on via the instance variable.
		 */
		@Override
		public void setup(Context context) {
			try {
				Configuration conf = context.getConfiguration();
				Path cachedDenomPath = DistributedCache
						.getLocalCacheFiles(conf)[0];
				BufferedReader reader = new BufferedReader(new FileReader(
						cachedDenomPath.toString()));
				String denomStr = reader.readLine();
				reader.close();
				denom = Long.decode(denomStr);
			} catch (IOException ioe) {
				System.err
						.println("IOException reading denom from distributed cache");
				System.err.println(ioe.toString());
			}
		}

		/**
		 * Randomly select vertices as starting points and output a
		 * DistanceValue for those. Then group outgoing edges together and store
		 * successors in DestinationsValue.
		 */
		@Override
		public void reduce(LongWritable key, Iterable<LongWritable> values,
				Context context) throws IOException, InterruptedException {
			//Wraps output
			EValue outWrapper = new EValue();

			// Randomly select this key as a starting vertex: 0 distance indicates start.
			if (Math.random() < 1.0 / denom) {
				context.getCounter(GraphCounter.ORIGINS).increment(1);
				outWrapper.set(new DistanceValue(0, key.get(), true));
				context.write(key, outWrapper);
			}

			// Group successors together.
			ArrayList<Long> destinations = new ArrayList<Long>();
			for (LongWritable val : values) {
				destinations.add(val.get());
			}
			long[] primitiveIds = new long[destinations.size()];
			for (int i = 0; i < primitiveIds.length; i++) {
				primitiveIds[i] = destinations.get(i);
			}
			outWrapper.set(new DestinationsValue(primitiveIds));
			context.write(key, outWrapper);
		}
	}

	/** Shares denom argument across the cluster via DistributedCache. */
	public static void shareDenom(String denomStr, Configuration conf) {
		try {
			Path localDenomPath = new Path(DENOM_PATH + "-source");
			Path remoteDenomPath = new Path(DENOM_PATH);
			BufferedWriter writer = new BufferedWriter(new FileWriter(
					localDenomPath.toString()));
			writer.write(denomStr);
			writer.newLine();
			writer.close();
			FileSystem fs = FileSystem.get(conf);
			fs.copyFromLocalFile(true, true, localDenomPath, remoteDenomPath);
			DistributedCache.addCacheFile(remoteDenomPath.toUri(), conf);
		} catch (IOException ioe) {
			System.err.println("IOException writing to distributed cache");
			System.err.println(ioe.toString());
		}
	}

	/** Identity map. */
	public static class SearchMap extends
			Mapper<LongWritable, EValue, LongWritable, EValue> {
		@Override
		public void map(LongWritable key, EValue value, Context context)
				throws IOException, InterruptedException {
			context.write(key, value);
		}
	}

	/** Propagation of breadth-first search. */
	public static class SearchReduce extends
			Reducer<LongWritable, EValue, LongWritable, EValue> {

		/*
		 * Updates distances by adding one to the appropriate distance of each
		 * successor. (If a distance already has been propagated,
		 * we can skip this step) For example:
		 * 
		 * A -> B -> C -> D
		 * 
		 * B is distance 1 from A. So we know that C must be 1 more away from A
		 * than B.
		 * 
		 * After ea. iteration, a pair of points say (E, F) might have several
		 * distances say 4, 8, 10, 12. The purpose of the HashMap is to store
		 * the minimum distance each time.
		 */
		@Override
		public void reduce(LongWritable key, Iterable<EValue> values,
				Context context) throws IOException, InterruptedException {
			//Wraps output
			EValue outWrapper = new EValue();
			// keeps track of all successors for this vertex (key)
			long[] destinations = null;
			// distances maps from ea. origin to distance(key, origin)
			HashMap<Long, Integer> distances = new HashMap<Long, Integer>();
			// Origins that still need to have distance propagated to successors.
			HashMap<Long, Boolean> needToPropagate = new HashMap<Long, Boolean>();

			/* Calculate min. distances, remember destinations, and note which distances
			need to be propagated. */
			for (EValue wrapper : values) {
				Writable val = wrapper.get();
				if (val instanceof DestinationsValue) {
					destinations = ((DestinationsValue) val).getDestinations();
				} else if (val instanceof DistanceValue) {
					DistanceValue dVal = (DistanceValue) val;
					// take minimum distance ea. time
					long origin = dVal.getOrigin();
					int distance = dVal.getDistance();
					if (distances.containsKey(origin)) {
						if (distances.get(origin) > distance) {
							distances.put(origin, distance);
						}
					} else {
						distances.put(origin, distance);
					}
					/*
					 * Propagate distance for an origin only if that distance
					 * has been found, and no pairs indicate distance has
					 * already been propagated.
					 */
					if (dVal.needDistancePropagated()
							&& !needToPropagate.containsKey(origin)) {
						needToPropagate.put(origin, true);
					} else {
						needToPropagate.put(origin, false);
					}
				} else {
					throw new IOException("Unknown value " + val.getClass());
				}
			}

			/*
			 * Emit updated distances (the min. possible). Also mark that we
			 * will have propagated distances for these particular origins in
			 * distances, so set a flag that won't need to propagate any more.
			 */
			for (Map.Entry<Long, Integer> pairing : distances.entrySet()) {
				outWrapper.set(new DistanceValue(pairing.getValue(), pairing.getKey(), false));
				context.write(key, outWrapper);
			}

			/*
			 * Update distance for successors which have not had distances
			 * propagated yet, but current vertex has a distance value for that
			 * origin. Emit (destination, DISTANCE distance+1 origin_id
			 * not_yet_propagated)
			 */
			for (Map.Entry<Long, Boolean> pair: needToPropagate.entrySet()) {
				if (pair.getValue()) {
					long origin = pair.getKey();
					int distance = distances.get(origin);
					for (long destination : destinations) {
						outWrapper.set(new DistanceValue(distance + 1, origin, true));
						context.write(new LongWritable(destination), outWrapper);
					}
				}
			}

			// Emit destinations if not all distances have been found
			if (distances.size() < _origins) {
				context.getCounter(GraphCounter.DISTANCES_FOUND).increment(distances.size());
				outWrapper.set(new DestinationsValue(destinations));
				context.write(key, outWrapper);
			}
		}
	}

	/** Just keep distance information. */
	public static class CleanupMap extends
			Mapper<LongWritable, EValue, LongWritable, DistanceValue> {
		@Override
		public void map(LongWritable key, EValue value, Context context)
				throws IOException, InterruptedException {
			Writable unwrapped = value.get();
			if (unwrapped instanceof DistanceValue) {
				context.write(key, (DistanceValue) unwrapped);
			}
		}
	}

	/** Output (distance, 1) for each origin of each key. */
	public static class CleanupReduce extends
			Reducer<LongWritable, DistanceValue, LongWritable, LongWritable> {

		public static LongWritable ONE = new LongWritable(1L);

		@Override
		public void reduce(LongWritable key, Iterable<DistanceValue> values,
				Context context) throws IOException, InterruptedException {
			// distances maps from ea. origin to distance(key, origin)
			HashMap<Long, Integer> distances = new HashMap<Long, Integer>();
			for (DistanceValue dVal : values) {
				// take minimum distance ea. time
				long origin = dVal.getOrigin();
				int distance = dVal.getDistance();
				if (distances.containsKey(origin)) {
					if (distances.get(origin) > distance) {
						distances.put(origin, distance);
					}
				} else {
					distances.put(origin, distance);
				}
			}
			for (Integer distance : distances.values()) {
				context.write(new LongWritable(distance), ONE);
			}
		}
	}

	/** Identity map. */
	public static class HistogramMap extends
			Mapper<LongWritable, LongWritable, LongWritable, LongWritable> {
		@Override
		public void map(LongWritable key, LongWritable value, Context context)
				throws IOException, InterruptedException {
			context.write(key, value);
		}
	}

	/** Total counts for ea. distance value. */
	public static class HistogramReduce extends
			Reducer<LongWritable, LongWritable, LongWritable, LongWritable> {
		@Override
		public void reduce(LongWritable key, Iterable<LongWritable> values,
				Context context) throws IOException, InterruptedException {
			long sum = 0L;
			for (LongWritable val : values) {
				sum += 1L;
			}
			context.write(key, new LongWritable(sum));
		}
	}

	public static void main(String[] rawArgs) throws Exception {
		// measure time taken
		long startTime = System.currentTimeMillis();

		GenericOptionsParser parser = new GenericOptionsParser(rawArgs);
		Configuration conf = parser.getConfiguration();
		String[] args = parser.getRemainingArgs();

		// Set denom from command line arguments
		shareDenom(args[2], conf);

		// Setting up mapreduce job to load in graph
		Job job = new Job(conf, "load graph");
		job.setJarByClass(SmallWorld.class);

		job.setMapOutputKeyClass(LongWritable.class);
		job.setMapOutputValueClass(LongWritable.class);
		job.setOutputKeyClass(LongWritable.class);
		job.setOutputValueClass(EValue.class);

		job.setMapperClass(LoaderMap.class);
		job.setReducerClass(LoadReduce.class);

		job.setInputFormatClass(SequenceFileInputFormat.class);
		job.setOutputFormatClass(SequenceFileOutputFormat.class);

		// Input from command-line argument, output to predictable place
		FileInputFormat.addInputPath(job, new Path(args[0]));
		FileOutputFormat.setOutputPath(job, new Path("bfs-0-out"));

		// Actually starts job, and waits for it to finish
		job.waitForCompletion(true);

		// Loads number of origins
		_origins = job.getCounters().findCounter(GraphCounter.ORIGINS).getValue();

		// Repeats your BFS mapreduce
		int i = 0;
		long prevDistancesFound = -1;
		while (i < MAX_ITERATIONS) {
			job = new Job(conf, "bfs" + i);
			job.setJarByClass(SmallWorld.class);

			job.setMapOutputKeyClass(LongWritable.class);
			job.setMapOutputValueClass(EValue.class);
			job.setOutputKeyClass(LongWritable.class);
			job.setOutputValueClass(EValue.class);

			job.setMapperClass(SearchMap.class);
			job.setReducerClass(SearchReduce.class);

			job.setInputFormatClass(SequenceFileInputFormat.class);
			job.setOutputFormatClass(SequenceFileOutputFormat.class);

			// Notice how each mapreduce job gets gets its own output dir
			FileInputFormat.addInputPath(job, new Path("bfs-" + i + "-out"));
			FileOutputFormat.setOutputPath(job, new Path("bfs-" + (i + 1)
					+ "-out"));

			job.waitForCompletion(true);
			i++;
			long distancesFound =
					job.getCounters().findCounter(GraphCounter.DISTANCES_FOUND).getValue();
			if (prevDistancesFound == distancesFound) {
				break;
			} else {
				prevDistancesFound = distancesFound;
			}
		}

		// Cleanup
		job = new Job(conf, "cleanup");
		job.setJarByClass(SmallWorld.class);

		job.setMapOutputKeyClass(LongWritable.class);
		job.setMapOutputValueClass(DistanceValue.class);
		job.setOutputKeyClass(LongWritable.class);
		job.setOutputValueClass(LongWritable.class);

		job.setMapperClass(CleanupMap.class);
		job.setReducerClass(CleanupReduce.class);

		job.setInputFormatClass(SequenceFileInputFormat.class);
		job.setOutputFormatClass(SequenceFileOutputFormat.class);

		FileInputFormat.addInputPath(job, new Path("bfs-" + i + "-out"));
		FileOutputFormat
				.setOutputPath(job, new Path("bfs-" + (i + 1) + "-out"));

		job.waitForCompletion(true);
		i++;

		// Mapreduce config for histogram computation
		job = new Job(conf, "hist");
		job.setJarByClass(SmallWorld.class);

		job.setMapOutputKeyClass(LongWritable.class);
		job.setMapOutputValueClass(LongWritable.class);
		job.setOutputKeyClass(LongWritable.class);
		job.setOutputValueClass(LongWritable.class);

		job.setMapperClass(HistogramMap.class);
		job.setReducerClass(HistogramReduce.class);

		job.setInputFormatClass(SequenceFileInputFormat.class);
		job.setOutputFormatClass(TextOutputFormat.class);

		// By declaring i above outside of loop conditions, can use it
		// here to get last bfs output to be input to histogram
		FileInputFormat.addInputPath(job, new Path("bfs-" + i + "-out"));
		FileOutputFormat.setOutputPath(job, new Path(args[1]));

		job.waitForCompletion(true);

		System.out.printf("\n\n");
		System.out.printf("%d origins selected\n\n", _origins);
		System.out.printf("%3.3fs elapsed\n", (System.currentTimeMillis() - startTime) / 1000.0);
		System.out.printf("\n\n");
	}
}
